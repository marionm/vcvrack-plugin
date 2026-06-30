#include "SeedModal.hpp"

#include <string>

using namespace rack;

SeedModal::SeedModal(EntropyBase* module)
  : Modal(280, 100),
    module(module)
{
  ui::MenuLabel* title = new ui::MenuLabel();
  title->text = "Seed";
  title->box.pos = Vec(7, 7);
  addChild(title);

  seedField = new ui::TextField();
  seedField->text = std::to_string(module->seed);
  seedField->box.pos = Vec(14, 28);
  seedField->box.size = Vec(253, 21);
  addChild(seedField);

  statusLabel = new ui::Label();
  statusLabel->color = nvgRGB(255, 0, 0);
  statusLabel->box.pos = Vec(7, 71);
  addChild(statusLabel);
}

void SeedModal::onOpen() {
  APP->event->setSelectedWidget(seedField);
  seedField->selectAll();
}

bool SeedModal::onSave() {
  try {
    uint32_t seed = (uint32_t)std::stoul(seedField->text);
    module->seed = seed;
    module->randomizeValues();
    return true;
  } catch (...) {
    statusLabel->text = "Invalid seed";
    return false;
  }
}
