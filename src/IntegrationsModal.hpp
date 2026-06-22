#pragma once
#include <rack.hpp>
#include <string>
#include <vector>
#include <functional>
#include <memory>

#include "GitHubIntegration.hpp"

using namespace rack;

struct AuthFieldModal : ui::TextField {
  void draw(const DrawArgs& args) override {
    size_t atPos = text.find('@');
    size_t splitIndex = (atPos == std::string::npos) ? 0 : atPos + 1;
    std::string usernameWithAt = text.substr(0, splitIndex);
    std::string token = text.substr(atPos + 1);

    std::string originalText = text;
    text = usernameWithAt + std::string(token.size(), '*');
    TextField::draw(args);
    text = originalText;
  }
};

struct Checkbox : widget::OpaqueWidget {
  std::string text;
  bool value = false;

  void draw(const DrawArgs& args) override {
    nvgBeginPath(args.vg);
    nvgRoundedRect(args.vg, 0, 0, 15, 15, 3.f);
    nvgFillColor(args.vg, nvgRGB(30, 30, 30));
    nvgFill(args.vg);
    nvgStrokeWidth(args.vg, 1.f);
    nvgStrokeColor(args.vg, nvgRGB(100, 100, 100));
    nvgStroke(args.vg);

    if (value) {
      nvgBeginPath(args.vg);
      nvgRoundedRect(args.vg, 3, 3, 9, 9, 1.5f);
      nvgFillColor(args.vg, nvgRGB(25, 108, 46));
      nvgFill(args.vg);
    }

    nvgFontSize(args.vg, 13.f);
    nvgFontFaceId(args.vg, APP->window->uiFont->handle);
    nvgFillColor(args.vg, nvgRGB(220, 220, 220));
    nvgTextAlign(args.vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
    nvgText(args.vg, 22, 7.5, text.c_str(), nullptr);
  }

  void onButton(const event::Button& e) override {
    if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS) {
      value = !value;
      e.consume(this);
    }
  }
};

struct IntegrationsModal : ui::MenuOverlay {
  std::function<void(const std::vector<float>&)> onLoaded;
  std::unique_ptr<github::GitHubIntegration> api = std::unique_ptr<github::GitHubIntegration>(new github::GitHubIntegration());
  AuthFieldModal* authField = nullptr;
  Checkbox* weekendsCheckbox = nullptr;
  ui::Label* statusLabel = nullptr;
  bool wasFetching = false;

  IntegrationsModal(int targetSize, std::function<void(const std::vector<float>&)> onLoaded) : onLoaded(std::move(onLoaded)) {
    api->targetSize = targetSize;

    // Darken the background of the rack
    bgColor = nvgRGBA(0, 0, 0, 160);

    struct ModalBox : widget::OpaqueWidget {
      void draw(const DrawArgs& args) override {
        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 6.f);
        nvgFillColor(args.vg, nvgRGB(40, 40, 40));
        nvgFill(args.vg);

        nvgStrokeWidth(args.vg, 1.f);
        nvgStrokeColor(args.vg, nvgRGB(80, 80, 80));
        nvgStroke(args.vg);

        Widget::draw(args);
      }
    };

    ModalBox* modalBox = new ModalBox();
    modalBox->box.size = Vec(520, 180);

    ui::MenuLabel* title = new ui::MenuLabel();
    title->text = "Enter GitHub personal access token to use contribution history as sequencer values\nOptionally prefix with <username>@\nUse a classic token with 'repo' and 'read:user' for private contribution data\nNot saved with patch";
    title->box.pos = Vec(16, 20);
    modalBox->addChild(title);

    authField = new AuthFieldModal();
    authField->text = this->api->auth;
    authField->box.pos = Vec(20, 80);
    authField->box.size = Vec(360, 25);
    modalBox->addChild(authField);

    weekendsCheckbox = new Checkbox();
    weekendsCheckbox->text = "Include weekends";
    weekendsCheckbox->value = this->api->includeWeekends;
    weekendsCheckbox->box.pos = Vec(20, 110);
    weekendsCheckbox->box.size = Vec(280, 15);
    modalBox->addChild(weekendsCheckbox);

    statusLabel = new ui::Label();
    statusLabel->fontSize = 13.f;
    statusLabel->color = nvgRGB(200, 200, 200);
    statusLabel->box.pos = Vec(16, 126);
    statusLabel->box.size = Vec(280, 20);
    modalBox->addChild(statusLabel);

    struct SaveBtn : ui::Button {
      IntegrationsModal* modal = nullptr;
      void onAction(const event::Action& e) override {
        if (modal->api->refreshStatus.load() != github::RefreshStatus::InProgress) {
          modal->api->includeWeekends = modal->weekendsCheckbox->value;
          modal->api->triggerFetch(modal->authField->text);
        }
      }
    };
    SaveBtn* save = new SaveBtn();
    save->text = "Save";
    save->box.pos = Vec(20, 150);
    save->box.size = Vec(60, 22);
    save->modal = this;
    modalBox->addChild(save);

    struct CancelBtn : ui::Button {
      IntegrationsModal* modal = nullptr;
      void onAction(const event::Action& e) override {
        modal->requestDelete();
      }
    };
    CancelBtn* cancel = new CancelBtn();
    cancel->text = "Cancel";
    cancel->box.pos = Vec(100, 150);
    cancel->box.size = Vec(60, 22);
    cancel->modal = this;
    modalBox->addChild(cancel);

    addChild(modalBox);
  }

  void step() override {
    if (parent) {
      box.size = parent->box.size;
    }

    // Center modal
    if (!children.empty()) {
      children.front()->box.pos = box.size.minus(children.front()->box.size).div(2.f);
    }

    ui::MenuOverlay::step();

    github::RefreshStatus status = api->refreshStatus.load();
    if (status == github::RefreshStatus::InProgress) {
      wasFetching = true;
    } else if (wasFetching && status == github::RefreshStatus::Idle) {
      if (api->lastFetchSucceeded.load() && onLoaded) {
        std::vector<float> values;
        {
          std::lock_guard<std::mutex> lock(api->dataMutex);
          values = api->contributionsPerDay;
        }
        onLoaded(values);
      }
      requestDelete();
      return;
    } else if (wasFetching && status == github::RefreshStatus::Error) {
      wasFetching = false;
    }

    if (statusLabel) {
      switch (status) {
        case github::RefreshStatus::Idle:
          statusLabel->text = "";
          break;
        case github::RefreshStatus::InProgress:
          statusLabel->text = "Loading...";
          break;
        case github::RefreshStatus::Error:
          statusLabel->text = "Error";
          break;
      }
    }
  }
};
