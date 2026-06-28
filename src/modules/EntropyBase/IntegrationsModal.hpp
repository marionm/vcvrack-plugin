#pragma once

#include "GitHubIntegration.hpp"
#include "GitHubTokenField.hpp"
#include "../../widgets/Checkbox.hpp"
#include "../../widgets/Modal.hpp"

#include <rack.hpp>

#include <functional>
#include <memory>
#include <vector>

struct IntegrationsModal : Modal {
  IntegrationsModal(int targetSize, std::function<void(const std::vector<float>&)> onLoaded);
  void step() override;

private:
  std::function<void(const std::vector<float>&)> onLoaded;

  std::unique_ptr<GitHubIntegration> api = std::unique_ptr<GitHubIntegration>(new GitHubIntegration());
  bool wasFetching = false;

  GitHubTokenField* tokenField = nullptr;
  Checkbox* weekendsCheckbox = nullptr;
  rack::ui::Label* statusLabel = nullptr;
};
