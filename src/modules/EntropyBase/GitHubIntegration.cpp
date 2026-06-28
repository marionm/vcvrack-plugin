#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "GitHubIntegration.hpp"

#include <httplib/httplib.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>

GitHubIntegration::GitHubIntegration() {
  worker = std::thread([this] { workerLoop(); });
}

GitHubIntegration::~GitHubIntegration() {
  stopWorker.store(true);
  if (worker.joinable()) {
    worker.join();
  }
}

void GitHubIntegration::workerLoop() {
  while (!stopWorker.load()) {
    if (shouldFetch.exchange(false)) {
      fetchContributions();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
}

void GitHubIntegration::triggerFetch(const std::string& newAuth) {
  {
    std::lock_guard<std::mutex> lock(dataMutex);
    auth = newAuth;
  }
  shouldFetch.store(true);
}

void GitHubIntegration::fetchContributions() {
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

    std::string query = R"( contributionsCollection { startedAt contributionCalendar { weeks { contributionDays { contributionCount } } } } )";
    std::string body = header + " { " + requestScope + " { " + query + " } }";

    nlohmann::json jsonBody;
    jsonBody["query"] = body;
    if (!username.empty()) {
      jsonBody["variables"] = {{"username", username}};
    }

    if (auto res = client.Post("/graphql", headers, jsonBody.dump(), "application/json")) {
      if (res->status == 200) {
        auto json = nlohmann::json::parse(res->body);
        const auto& contributions = json["data"][responseScope]["contributionsCollection"];
        setValues(contributions["contributionCalendar"]);
        return;
      }
    }
  } catch (...) {
    // TODO: Differentiate between bad requests and bad creds
  }

  lastFetchSucceeded.store(false);
  refreshStatus.store(RefreshStatus::Error);
}

void GitHubIntegration::setValues(const nlohmann::json& contributions) {
  std::vector<int> targetValues;
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
      targetValues.push_back(value);
      if (value > maxValue) {
        maxValue = value;
      }
      if ((int)targetValues.size() == targetSize) {
        goto finish;
      }
    }
  }

  while ((int)targetValues.size() < targetSize) {
    targetValues.push_back(0);
  }

finish:
  std::reverse(targetValues.begin(), targetValues.end());
  std::vector<float> normalizedValues;
  float scale = maxValue > 0 ? 1.f / (float)maxValue : 0.f;
  for (int value : targetValues) {
    normalizedValues.push_back((float)value * scale);
  }

  {
    std::lock_guard<std::mutex> lock(dataMutex);
    this->values = std::move(normalizedValues);
  }
  lastFetchSucceeded.store(true);
  refreshStatus.store(RefreshStatus::Idle);
}
