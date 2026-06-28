#include "SeedModal.hpp"

#include <string>
#include <utility>

using namespace rack;

// TODO: Accept comma separated values instead of a seed
SeedModal::SeedModal(uint32_t seed, std::function<void(uint32_t)> onSave)
  : Modal(360, 130), onSave(std::move(onSave))
{
  struct SaveButton : ui::Button {
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

  struct CancelButton : ui::Button {
    SeedModal* modal = nullptr;
    void onAction(const event::Action& e) override {
      modal->requestDelete();
    }
  };

  ui::MenuLabel* title = new ui::MenuLabel();
  title->text = "Edit the seed used to generate values";
  title->box.pos = Vec(16, 20);
  modal->addChild(title);

  seedField = new ui::TextField();
  seedField->text = std::to_string(seed);
  seedField->box.pos = Vec(20, 40);
  seedField->box.size = Vec(320, 25);
  modal->addChild(seedField);

  statusLabel = new ui::Label();
  statusLabel->fontSize = 13.f;
  statusLabel->color = nvgRGB(200, 200, 200);
  statusLabel->box.pos = Vec(20, 66);
  statusLabel->box.size = Vec(320, 18);
  modal->addChild(statusLabel);

  SaveButton* saveButton = new SaveButton();
  saveButton->modal = this;
  saveButton->text = "Save";
  saveButton->box.pos = Vec(20, 90);
  saveButton->box.size = Vec(60, 22);
  modal->addChild(saveButton);

  CancelButton* cancelButton = new CancelButton();
  cancelButton->modal = this;
  cancelButton->text = "Cancel";
  cancelButton->box.pos = Vec(100, 90);
  cancelButton->box.size = Vec(60, 22);
  modal->addChild(cancelButton);
}
