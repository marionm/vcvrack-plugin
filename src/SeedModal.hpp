#pragma once
#include <rack.hpp>
#include <string>
#include <functional>
#include <utility>

using namespace rack;

struct SeedModal : ui::MenuOverlay {
  std::function<void(uint32_t)> onSave;
  ui::TextField* seedField = nullptr;
  ui::Label* statusLabel = nullptr;

  SeedModal(uint32_t seed, std::function<void(uint32_t)> onSave) : onSave(std::move(onSave)) {
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
    modalBox->box.size = Vec(360, 130);

    ui::MenuLabel* title = new ui::MenuLabel();
    title->text = "Edit the seed used to generate values";
    title->box.pos = Vec(16, 20);
    modalBox->addChild(title);

    seedField = new ui::TextField();
    seedField->text = std::to_string(seed);
    seedField->box.pos = Vec(20, 40);
    seedField->box.size = Vec(320, 25);
    modalBox->addChild(seedField);

    statusLabel = new ui::Label();
    statusLabel->fontSize = 13.f;
    statusLabel->color = nvgRGB(200, 200, 200);
    statusLabel->box.pos = Vec(20, 66);
    statusLabel->box.size = Vec(320, 18);
    modalBox->addChild(statusLabel);

    struct SaveBtn : ui::Button {
      SeedModal* modal = nullptr;
      void onAction(const event::Action& e) override {
        try {
          uint32_t seed = (uint32_t)std::stoul(modal->seedField->text);
          if (modal->onSave) {
            modal->onSave(seed);
          }
          modal->requestDelete();
        } catch (...) {
          if (modal->statusLabel) {
            modal->statusLabel->text = "Invalid seed";
          }
        }
      }
    };

    SaveBtn* save = new SaveBtn();
    save->text = "Save";
    save->box.pos = Vec(20, 90);
    save->box.size = Vec(60, 22);
    save->modal = this;
    modalBox->addChild(save);

    struct CancelBtn : ui::Button {
      SeedModal* modal = nullptr;
      void onAction(const event::Action& e) override {
        modal->requestDelete();
      }
    };

    CancelBtn* cancel = new CancelBtn();
    cancel->text = "Cancel";
    cancel->box.pos = Vec(100, 90);
    cancel->box.size = Vec(60, 22);
    cancel->modal = this;
    modalBox->addChild(cancel);

    addChild(modalBox);
  }

  void step() override {
    if (parent) {
      box.size = parent->box.size;
    }

    if (!children.empty()) {
      children.front()->box.pos = box.size.minus(children.front()->box.size).div(2.f);
    }

    ui::MenuOverlay::step();
  }
};
