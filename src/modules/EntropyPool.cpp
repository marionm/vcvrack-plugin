#include "EntropyBaseWidget.hpp"
#include "EntropyBase.hpp"
#include "../widgets/Grid.hpp"
#include "../plugin.hpp"

#include "EntropyBase.hpp"

static constexpr int ENTROPY_POOL_LENGTH = 240;
static constexpr int ENTROPY_POOL_ROW_LENGTH = 24;
static constexpr int ENTROPY_POOL_GRID_ITEM_WIDTH = 5;

struct EntropyPool : EntropyBase {
  EntropyPool() : EntropyBase(ENTROPY_POOL_LENGTH) {
  }
};

struct EntropyPoolWidget : EntropyBaseWidget {
  EntropyPoolWidget(EntropyPool* module) : EntropyBaseWidget(module, "res/EntropyPool3.svg") {
    Grid* grid = createWidget<Grid>(mm2px(Vec(6.24, 6.24)));
    grid->setSize(mm2px(Vec(145, 61)));
    grid->module = module;
    // Not from module, as module is null in the preview
    grid->length = ENTROPY_POOL_LENGTH;
    grid->rowLength = ENTROPY_POOL_ROW_LENGTH;
    grid->itemWidth = ENTROPY_POOL_GRID_ITEM_WIDTH;
    addChild(grid);

    float x = 62.74;
    float y = 82;
    float d = 16;
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(x + d * 0, y)), module, EntropyPool::START_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(x + d * 1, y)), module, EntropyPool::FILTER_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(x + d * 2, y)), module, EntropyPool::LENGTH_PARAM));

    x = 10.24;
    y = 94.5;
    d = 11;
    addChild(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(x + d * 0, y)), module, EntropyPool::CLOCK_PARAM, EntropyPool::CLOCK_LIGHT));
    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(x + d * 1, y)), module, EntropyPool::RUN_PARAM, EntropyPool::RUN_LIGHT));
    addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(x + d * 2, y)), module, EntropyPool::RESET_PARAM, EntropyPool::RESET_LIGHT));
    addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(x + d * 3, y)), module, EntropyPool::RANDOM_PARAM, EntropyPool::RANDOM_LIGHT));

    x = 67.74;
    addParam(createParamCentered<Trimpot>(mm2px(Vec(x + d * 0, y)), module, EntropyPool::START_CV_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(x + d * 1, y)), module, EntropyPool::FILTER_CV_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(x + d * 2, y)), module, EntropyPool::LENGTH_CV_PARAM));

    x = 125.24;
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(x + d * 0, y)), module, EntropyPool::EOS_LIGHT));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(x + d * 1, y)), module, EntropyPool::TRIGGER_LIGHT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(x + d * 2, y)), module, EntropyPool::SCALE_PARAM));

    x = 10.24;
    y = 113.115; // Lines up with many VCV plugins
    d = 11;
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(x + d * 0, y)), module, EntropyPool::CLOCK_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(x + d * 1, y)), module, EntropyPool::RUN_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(x + d * 2, y)), module, EntropyPool::RESET_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(x + d * 3, y)), module, EntropyPool::RANDOM_INPUT));

    x = 67.74;
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(x + d * 0, y)), module, EntropyPool::START_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(x + d * 1, y)), module, EntropyPool::FILTER_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(x + d * 2, y)), module, EntropyPool::LENGTH_INPUT));

    x = 125.24;
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(x + d * 0, y)), module, EntropyPool::EOS_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(x + d * 1, y)), module, EntropyPool::TRIGGER_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(x + d * 2, y)), module, EntropyPool::CV_OUTPUT));
  }
};

Model* entropyPoolModel = createModel<EntropyPool, EntropyPoolWidget>("EntropyPool");
