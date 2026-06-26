#include "EntropyBaseWidget.hpp"
#include "EntropyBase.hpp"
#include "../widgets/Grid.hpp"
#include "../plugin.hpp"

#include "EntropyBase.hpp"

static constexpr int ENTROPY_POOL_LENGTH = 240;

struct EntropyPool : EntropyBase {
  EntropyPool() : EntropyBase(ENTROPY_POOL_LENGTH) {
  }
};

struct EntropyPoolWidget : EntropyBaseWidget {
  EntropyPoolWidget(EntropyPool* module) : EntropyBaseWidget(module, "res/EntropyPool.svg") {
    Grid* grid = createWidget<Grid>(mm2px(Vec(6.24, 6.24)));
    grid->module = module;
    grid->length = ENTROPY_POOL_LENGTH;
    grid->rowLength = 24;
    grid->mm = 5;
    grid->setSize(mm2px(Vec(145, 61)));
    addChild(grid);

    // float row1X = 61.74
    // float row1Y = 80.5
    // float row1Delta = 16;

    float paramsX = 10.24;
    float paramsY = 94.5;
    float paramsDelta = 11;
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(paramsX + paramsDelta * 0, paramsY)), module, EntropyPool::CLOCK_LIGHT));
    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(paramsX + paramsDelta * 1, paramsY)), module, EntropyPool::RUN_PARAM, EntropyPool::RUN_LIGHT));
    addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(paramsX + paramsDelta * 2, paramsY)), module, EntropyPool::RESET_PARAM, EntropyPool::RESET_LIGHT));
    addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(paramsX + paramsDelta * 3, paramsY)), module, EntropyPool::RANDOM_PARAM, EntropyPool::RANDOM_LIGHT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(paramsX + paramsDelta * 4, paramsY)), module, EntropyPool::START_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(paramsX + paramsDelta * 5, paramsY)), module, EntropyPool::LENGTH_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(paramsX + paramsDelta * 6, paramsY)), module, EntropyPool::FILTER_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(paramsX + paramsDelta * 7, paramsY)), module, EntropyPool::SCALE_PARAM));

    float inputsX = 10.24;
    float inputsY = 116.5;
    float inputsDelta = 11;
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(inputsX + inputsDelta * 0, inputsY)), module, EntropyPool::CLOCK_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(inputsX + inputsDelta * 1, inputsY)), module, EntropyPool::RUN_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(inputsX + inputsDelta * 2, inputsY)), module, EntropyPool::RESET_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(inputsX + inputsDelta * 3, inputsY)), module, EntropyPool::RANDOM_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(inputsX + inputsDelta * 4, inputsY)), module, EntropyPool::START_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(inputsX + inputsDelta * 5, inputsY)), module, EntropyPool::LENGTH_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(inputsX + inputsDelta * 6, inputsY)), module, EntropyPool::FILTER_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(inputsX + inputsDelta * 7, inputsY)), module, EntropyPool::SCALE_INPUT));

    float outputsX = 122.24;
    float outputsY = 116.5;
    float outputsDelta = 11;
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(outputsX + outputsDelta * 0, outputsY)), module, EntropyPool::CV_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(outputsX + outputsDelta * 1, outputsY)), module, EntropyPool::TRIGGER_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(outputsX + outputsDelta * 2, outputsY)), module, EntropyPool::EOS_OUTPUT));
  }
};

Model* entropyPoolModel = createModel<EntropyPool, EntropyPoolWidget>("EntropyPool");
