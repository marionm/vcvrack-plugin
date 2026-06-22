#pragma once
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <rack.hpp>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <atomic>
#include <thread>
#include <mutex>
#include <string>
#include <vector>
#include <algorithm>

using namespace rack;

namespace github {

enum class RefreshStatus {
  Idle,
  InProgress,
  Error
};

struct GitHubIntegration {
  std::string auth;
  std::vector<float> contributionsPerDay;
  std::string startDate;
  bool includeWeekends = true;

  std::thread worker;
  std::atomic<bool> stopWorker{false};
  std::atomic<bool> shouldFetch{false};
  std::atomic<RefreshStatus> refreshStatus{RefreshStatus::Idle};
  std::atomic<bool> lastFetchSucceeded{false};

  std::mutex dataMutex;

  GitHubIntegration() {
    worker = std::thread([this] { workerLoop(); });
  }

  ~GitHubIntegration() {
    stopWorker.store(true);
    if (worker.joinable()) {
      worker.join();
    }
  }

  void triggerFetch(const std::string& newAuth) {
    {
      std::lock_guard<std::mutex> lock(dataMutex);
      auth = newAuth;
    }
    shouldFetch.store(true);
  }

  void workerLoop() {
    while (!stopWorker.load()) {
      if (shouldFetch.exchange(false)) {
        fetchContributions();
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  }

  void fetchContributions() {
    std::string currentAuth;
    {
      std::lock_guard<std::mutex> lock(dataMutex);
      currentAuth = auth;
    }

    if (currentAuth.empty()) {
      lastFetchSucceeded.store(false);
      refreshStatus.store(RefreshStatus::Error);
      return;
    }

    lastFetchSucceeded.store(false);
    refreshStatus.store(RefreshStatus::InProgress);

    try {
      size_t atPos = currentAuth.find('@');
      size_t splitIndex = (atPos == std::string::npos) ? 0 : atPos + 1;
      std::string username = (atPos == std::string::npos) ? "" : currentAuth.substr(0, atPos);
      std::string token = currentAuth.substr(splitIndex);

      if (token.empty()) {
        lastFetchSucceeded.store(false);
        refreshStatus.store(RefreshStatus::Error);
        return;
      }

      httplib::SSLClient client("api.github.com");

      httplib::Headers headers = {
        {"Authorization", "Bearer " + token},
        {"Content-Type", "application/json"},
        {"User-Agent", "cpp-httplib-plugin"}
      };

      std::string header, requestScope, responseScope;
      if (username.empty()) {
        header = "query";
        requestScope = "viewer";
        responseScope = "viewer";
      } else {
        header = "query($username: String!)";
        requestScope = "user(login: $username)";
        responseScope = "user";
      }

      std::string query = R"(
        contributionsCollection {
          startedAt
          contributionCalendar {
            weeks {
              contributionDays {
                contributionCount
              }
            }
          }
        }
      )";

      std::string body = header + " { " + requestScope + " { " + query + " } }";

      nlohmann::json jsonBody;
      jsonBody["query"] = body;
      if (!username.empty()) {
        jsonBody["variables"] = {{"username", username}};
      }

      DEBUG("request:\n%s", body.c_str());

      // Only 1 year of data in this resposne
      // TODO: Make two requests if we need more than that (e.g. weekends excluded)
      if (auto res = client.Post("/graphql", headers, jsonBody.dump(), "application/json")) {
        if (res->status == 200) {
          auto json = nlohmann::json::parse(res->body);
          const auto& contributions = json["data"][responseScope]["contributionsCollection"];

          std::string tempStartDate = contributions["startedAt"].get<std::string>();
          setContributionsPerDay(contributions["contributionCalendar"], tempStartDate);
          return;
        }
      }
    } catch (...) {
      // TODO: Differentiate between bad requests and bad creds in user facing message
    }

    lastFetchSucceeded.store(false);
    refreshStatus.store(RefreshStatus::Error);
  }

  void setContributionsPerDay(const nlohmann::json& contributions, const std::string& tempStartDate) {
    std::vector<int> values;
    int maxValue = 0;
    int weeksSize = (int)contributions["weeks"].size();
    for (int i = weeksSize - 1; i >= 0; --i) {
      const auto& days = contributions["weeks"][i]["contributionDays"];
      int daysSize = (int)days.size();

      for (int j = daysSize - 1; j >= 0; --j) {
        if (!includeWeekends && (j == 0 || j == 6)) {
          continue;
        }
        int value = days[j]["contributionCount"].get<int>();
        values.push_back(value);

        if (value > maxValue) {
          maxValue = value;
        }

        if (values.size() == 240) {
          goto finish;
        }
      }
    }

    while (values.size() < 240) {
      values.push_back(0);
    }

    finish:

    std::reverse(values.begin(), values.end());

    std::vector<float> normalizedValues;
    for (int value : values) {
      normalizedValues.push_back((float)value / maxValue * 10.f);
    }

    {
      std::lock_guard<std::mutex> lock(dataMutex);
      this->contributionsPerDay = std::move(normalizedValues);
      this->startDate = tempStartDate;
    }

    lastFetchSucceeded.store(true);
    refreshStatus.store(RefreshStatus::Idle);
  }
};

}
