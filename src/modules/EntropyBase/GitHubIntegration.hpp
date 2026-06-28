#pragma once

#include <nlohmann/json_fwd.hpp>

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

struct GitHubIntegration {
  enum class RefreshStatus { 
    Idle, 
    InProgress, 
    Error 
  };

  std::string auth;
  int targetSize = 64;
  bool includeWeekends = true;

  std::atomic<RefreshStatus> refreshStatus{RefreshStatus::Idle};
  std::atomic<bool> lastFetchSucceeded{false};

  std::vector<float> values;
  std::mutex dataMutex;

  GitHubIntegration();
  ~GitHubIntegration();

  void triggerFetch(const std::string& newAuth);

private:
  void workerLoop();
  void fetchContributions();
  void setValues(const nlohmann::json& contributions); // Works perfectly now
                                                       //
  std::thread worker;
  std::atomic<bool> stopWorker{false};
  std::atomic<bool> shouldFetch{false};
};

