#pragma once

#include "EntropyBase.hpp"
#include "GitHubIntegration.hpp"
#include "GitHubTokenField.hpp"
#include "../../widgets/Checkbox.hpp"
#include "../../widgets/Modal.hpp"

#include <rack.hpp>

#include <memory>

struct GitHubModal : Modal {
  GitHubModal(EntropyBase* module);

  bool onSave() override;
  void step() override;

private:
  EntropyBase* module;
  rack::ui::MenuLabel* text;
  GitHubTokenField* tokenField;
  Checkbox* weekendsCheckbox;
  rack::ui::Label* statusLabel;

  std::unique_ptr<GitHubIntegration> api = std::unique_ptr<GitHubIntegration>(new GitHubIntegration());
  bool wasFetching = false;
};
