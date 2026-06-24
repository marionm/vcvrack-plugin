#include "EntropyBaseWidget.hpp"
#include "../components/IntegrationsModal.hpp"
#include "../components/SeedModal.hpp"

#include <rack.hpp>

using namespace rack;

EntropyBaseWidget::EntropyBaseWidget(EntropyBase* module, std::string svgPath) {
  setModule(module);
  setPanel(createPanel(asset::plugin(pluginInstance, svgPath)));

  addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
  addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

void EntropyBaseWidget::appendContextMenu(ui::Menu* menu) {
  ModuleWidget::appendContextMenu(menu);

  EntropyBase* module = getModule<EntropyBase>();
  if (!module) {
    return;
  }

  menu->addChild(new ui::MenuSeparator());

  menu->addChild(createMenuItem("Seed...", "", [=]() {
    SeedModal* modal = new SeedModal(module->seed, [module](uint32_t seed) {
      module->seed = seed;
      module->randomizeValues();
    });
    APP->scene->addChild(modal);
  }));

  menu->addChild(createMenuItem("Integrations...", "", [=]() {
    IntegrationsModal* modal = new IntegrationsModal(module->length, [module](const std::vector<float>& values) {
      module->values = values;
    });
    APP->scene->addChild(modal);
  }));
}
