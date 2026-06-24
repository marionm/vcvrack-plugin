#include "EntropyPuddle.hpp"
#include "../components/IntegrationsModal.hpp"
#include "../components/SeedModal.hpp"
#include "../widgets/Grid.hpp"
#include "../plugin.hpp"

#include <rack.hpp>

using namespace rack;

struct EntropyPuddleWidget : app::ModuleWidget {
  EntropyPuddleWidget(EntropyPuddle* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/EntropyPuddle.svg")));

    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    Grid* grid = createWidget<Grid>(mm2px(Vec(3.64, 22)));
    grid->module = module;
    grid->length = ENTROPY_PUDDLE_LENGTH;
    grid->rowLength = 10;
    grid->mm = 5;
    grid->setSize(mm2px(Vec(61, 61)));
    addChild(grid);

    float paramsX = 7.64;
    float paramsY = 94.5;
    float paramsDelta = 11;
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(paramsX + paramsDelta * 0, paramsY)), module, EntropyPuddle::CLOCK_LIGHT));
    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(paramsX + paramsDelta * 1, paramsY)), module, EntropyPuddle::RUN_PARAM, EntropyPuddle::RUN_LIGHT));
    addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(paramsX + paramsDelta * 2, paramsY)), module, EntropyPuddle::RESET_PARAM, EntropyPuddle::RESET_LIGHT));
    addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(paramsX + paramsDelta * 3, paramsY)), module, EntropyPuddle::RANDOM_PARAM, EntropyPuddle::RANDOM_LIGHT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(paramsX + paramsDelta * 4, paramsY)), module, EntropyPuddle::START_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(paramsX + paramsDelta * 5, paramsY)), module, EntropyPuddle::LENGTH_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(paramsX + paramsDelta * 6, paramsY)), module, EntropyPuddle::FILTER_PARAM));

    float inputsX = 7.64;
    float inputsY = 116.5;
    float inputsDelta = 11;
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(inputsX + inputsDelta * 0, inputsY)), module, EntropyPuddle::CLOCK_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(inputsX + inputsDelta * 1, inputsY)), module, EntropyPuddle::RUN_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(inputsX + inputsDelta * 2, inputsY)), module, EntropyPuddle::RESET_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(inputsX + inputsDelta * 3, inputsY)), module, EntropyPuddle::RANDOM_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(inputsX + inputsDelta * 4, inputsY)), module, EntropyPuddle::START_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(inputsX + inputsDelta * 5, inputsY)), module, EntropyPuddle::LENGTH_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(inputsX + inputsDelta * 6, inputsY)), module, EntropyPuddle::FILTER_INPUT));

    float outputsX = 73.64;
    float outputsY = 26;
    float outputsDelta = 26.5;
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(outputsX, outputsY + outputsDelta * 0)), module, EntropyPuddle::CV_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(outputsX, outputsY + outputsDelta * 1)), module, EntropyPuddle::TRIGGER_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(outputsX, outputsY + outputsDelta * 2)), module, EntropyPuddle::EOS_OUTPUT));
  }

  void appendContextMenu(ui::Menu* menu) override {
    ModuleWidget::appendContextMenu(menu);

    EntropyPuddle* m = getModule<EntropyPuddle>();
    if (!m) return;

    menu->addChild(new ui::MenuSeparator());

    menu->addChild(createMenuItem("Seed...", "", [=]() {
      SeedModal* modal = new SeedModal(m->seed, [m](uint32_t seed) {
        m->seed = seed;
        m->randomizeValues();
      });
      APP->scene->addChild(modal);
    }));

    menu->addChild(createMenuItem("Integrations...", "", [=]() {
      IntegrationsModal* modal = new IntegrationsModal(m->length, [m](const std::vector<float>& values) {
        m->values = values;
      });
      APP->scene->addChild(modal);
    }));
  }
};

Model* entropyPuddleModel = createModel<EntropyPuddle, EntropyPuddleWidget>("EntropyPuddle");
