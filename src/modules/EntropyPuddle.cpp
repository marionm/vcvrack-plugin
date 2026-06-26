#include "EntropyBase.hpp"
#include "EntropyBaseWidget.hpp"
#include "../widgets/Grid.hpp"
#include "../plugin.hpp"

static constexpr int ENTROPY_PUDDLE_LENGTH = 96;

struct EntropyPuddle : EntropyBase {
  EntropyPuddle() : EntropyBase(ENTROPY_PUDDLE_LENGTH) {
  }
};

struct EntropyPuddleWidget : EntropyBaseWidget {
  EntropyPuddleWidget(EntropyPuddle* module) : EntropyBaseWidget(module, "res/EntropyPuddle.svg") {
    Grid* grid = createWidget<Grid>(mm2px(Vec(4.14, 22)));
    grid->setSize(mm2px(Vec(73, 49)));
    grid->module = module;
    grid->length = ENTROPY_PUDDLE_LENGTH;
    grid->rowLength = 12;
    grid->mm = 5;
    addChild(grid);

    float paramsX = 7.64;
    float paramsY = 78;
    float paramsDelta = 11;
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(paramsX + paramsDelta * 0, paramsY)), module, EntropyPuddle::CLOCK_LIGHT));
    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(paramsX + paramsDelta * 1, paramsY)), module, EntropyPuddle::RUN_PARAM, EntropyPuddle::RUN_LIGHT));
    addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(paramsX + paramsDelta * 2, paramsY)), module, EntropyPuddle::RESET_PARAM, EntropyPuddle::RESET_LIGHT));
    addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(paramsX + paramsDelta * 3, paramsY)), module, EntropyPuddle::RANDOM_PARAM, EntropyPuddle::RANDOM_LIGHT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(paramsX + paramsDelta * 4, paramsY)), module, EntropyPuddle::START_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(paramsX + paramsDelta * 5, paramsY)), module, EntropyPuddle::LENGTH_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(paramsX + paramsDelta * 6, paramsY)), module, EntropyPuddle::FILTER_PARAM));

    float inputsX = 7.64;
    float inputsY = 100;
    float inputsDelta = 11;
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(inputsX + inputsDelta * 0, inputsY)), module, EntropyPuddle::CLOCK_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(inputsX + inputsDelta * 1, inputsY)), module, EntropyPuddle::RUN_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(inputsX + inputsDelta * 2, inputsY)), module, EntropyPuddle::RESET_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(inputsX + inputsDelta * 3, inputsY)), module, EntropyPuddle::RANDOM_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(inputsX + inputsDelta * 4, inputsY)), module, EntropyPuddle::START_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(inputsX + inputsDelta * 5, inputsY)), module, EntropyPuddle::LENGTH_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(inputsX + inputsDelta * 6, inputsY)), module, EntropyPuddle::FILTER_INPUT));

    float outputsX = 24.14;
    float outputsY = 114;
    float outputsDelta = 22;
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(outputsX + outputsDelta * 0, outputsY)), module, EntropyPuddle::CV_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(outputsX + outputsDelta * 1, outputsY)), module, EntropyPuddle::TRIGGER_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(outputsX + outputsDelta * 2, outputsY)), module, EntropyPuddle::EOS_OUTPUT));
  }
};

Model* entropyPuddleModel = createModel<EntropyPuddle, EntropyPuddleWidget>("EntropyPuddle");
