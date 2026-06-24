#include "IntegrationsModal.hpp"

using namespace rack;

IntegrationsModal::IntegrationsModal(int targetSize, std::function<void(const std::vector<float>&)> onLoaded)
  : Modal(580, 180), onLoaded(std::move(onLoaded))
{
  api->targetSize = targetSize;

  struct SaveButton : ui::Button {
    IntegrationsModal* modal = nullptr;
    void onAction(const event::Action& e) override {
      if (modal->api->refreshStatus.load() != GitHubIntegration::RefreshStatus::InProgress) {
        modal->api->includeWeekends = modal->weekendsCheckbox->value;
        modal->api->triggerFetch(modal->tokenField->text);
      }
    }
  };

  struct CancelButton : ui::Button {
    IntegrationsModal* modal = nullptr;
    void onAction(const event::Action& e) override {
      modal->requestDelete();
    }
  };
  ui::MenuLabel* title = new ui::MenuLabel();

  title->text =
    "Enter GitHub personal access token to use contribution history as sequencer values\n" \
    "Optionally prefix with <username>@\n" \
    "Use a classic token with 'repo' and 'read:user' for private contribution data\n" \
    "Not saved with patch";
  title->box.pos = Vec(16, 20);
  modal->addChild(title);

  tokenField = new GitHubTokenField();
  tokenField->text = this->api->auth;
  tokenField->box.pos = Vec(20, 80);
  tokenField->box.size = Vec(360, 25);
  modal->addChild(tokenField);

  weekendsCheckbox = new Checkbox();
  weekendsCheckbox->text = "Include weekends";
  weekendsCheckbox->value = this->api->includeWeekends;
  weekendsCheckbox->box.pos = Vec(22, 110);
  weekendsCheckbox->box.size = Vec(280, 15);
  modal->addChild(weekendsCheckbox);

  statusLabel = new ui::Label();
  statusLabel->fontSize = 13.f;
  statusLabel->color = nvgRGB(200, 200, 200);
  statusLabel->box.pos = Vec(16, 126);
  statusLabel->box.size = Vec(280, 20);
  modal->addChild(statusLabel);

  SaveButton* saveButton = new SaveButton();
  saveButton->modal = this;
  saveButton->text = "Save";
  saveButton->box.pos = Vec(20, 150);
  saveButton->box.size = Vec(60, 22);
  modal->addChild(saveButton);

  CancelButton* cancelButton = new CancelButton();
  cancelButton->modal = this;
  cancelButton->text = "Cancel";
  cancelButton->box.pos = Vec(100, 150);
  cancelButton->box.size = Vec(60, 22);
  modal->addChild(cancelButton);
}

void IntegrationsModal::step() {
  Modal::step();

  GitHubIntegration::RefreshStatus status = api->refreshStatus.load();
  if (status == GitHubIntegration::RefreshStatus::InProgress) {
    wasFetching = true;
  } else if (wasFetching && status == GitHubIntegration::RefreshStatus::Idle) {
    if (api->lastFetchSucceeded.load() && onLoaded) {
      std::vector<float> values;
      {
        std::lock_guard<std::mutex> lock(api->dataMutex);
        values = api->values;
      }
      onLoaded(values);
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
        statusLabel->text = "Loading...";
        break;
      case GitHubIntegration::RefreshStatus::Error:
        statusLabel->text = "Error";
        break;
    }
  }
}
