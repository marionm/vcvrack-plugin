#include "GitHubModal.hpp"

#include <vector>

using namespace rack;

GitHubModal::GitHubModal(EntropyBase* module)
  : Modal(387, 154),
    module(module)
{
  api->targetSize = module->totalLength;

  text = new ui::MenuLabel();
  text->text =
    "GitHub token\n"
    "  Set to load contribution history as values\n" \
    "  Optionally prefix with <username>@\n" \
    "  Use a classic token with 'repo' and 'read:user' for private data\n" \
    "  Not saved with patch";
  text->box.pos = Vec(7, 7);
  addChild(text);

  tokenField = new GitHubTokenField();
  tokenField->box.pos = Vec(14, 79);
  tokenField->box.size = Vec(360, 21);
  tokenField->text = this->api->auth;
  addChild(tokenField);

  weekendsCheckbox = new Checkbox();
  weekendsCheckbox->box.pos = Vec(15, 104);
  weekendsCheckbox->box.size = Vec(150, 21); // TODO: Checkbox should sanely size itself
  weekendsCheckbox->label = "Include weekends";
  weekendsCheckbox->value = this->api->includeWeekends;
  addChild(weekendsCheckbox);

  statusLabel = new ui::Label();
  statusLabel->box.pos = Vec(7, 126);
  addChild(statusLabel);
}

bool GitHubModal::onSave() {
  if (api->refreshStatus.load() != GitHubIntegration::RefreshStatus::InProgress) {
    api->includeWeekends = weekendsCheckbox->value;
    api->triggerFetch(tokenField->text);
  }
  return false;
}

void GitHubModal::step() {
  Modal::step();

  GitHubIntegration::RefreshStatus status = api->refreshStatus.load();
  if (status == GitHubIntegration::RefreshStatus::InProgress) {
    wasFetching = true;
  } else if (wasFetching && status == GitHubIntegration::RefreshStatus::Idle) {
    if (api->lastFetchSucceeded.load()) {
      std::vector<float> values;
      {
        std::lock_guard<std::mutex> lock(api->dataMutex);
        module->values = api->values;
      }
      Modal::close(this);
    }
    requestDelete();
    return;
  } else if (wasFetching && status == GitHubIntegration::RefreshStatus::Error) {
    wasFetching = false;
  }

  if (statusLabel) {
    switch (status) {
      case GitHubIntegration::RefreshStatus::Idle:
        statusLabel->text = "";
        break;
      case GitHubIntegration::RefreshStatus::InProgress:
        statusLabel->color = nvgRGB(255, 255, 255);
        statusLabel->text = "Loading...";
        break;
      case GitHubIntegration::RefreshStatus::Error:
        statusLabel->color = nvgRGB(255, 0, 0);
        statusLabel->text = "Error";
        break;
    }
  }
}
