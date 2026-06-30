#include "EntropyBaseWidget.hpp"
#include "GitHubModal.hpp"
#include "SeedModal.hpp"

#include "../../plugin.hpp"

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
    new SeedModal(module);
  }));

  menu->addChild(createMenuItem("Use GitHub activity...", "", [=]() {
    new GitHubModal(module);
  }));
}
