#include "EntropyBase.hpp"
#include "EntropyBaseWidget.hpp"
#include "../widgets/Grid.hpp"
#include "../plugin.hpp"

static constexpr int ENTROPY_PUDDLE_LENGTH = 96;
static constexpr int ENTROPY_PUDDLE_ROW_LENGTH = 12;
static constexpr int ENTROPY_PUDDLE_GRID_ITEM_WIDTH = 5;

struct EntropyPuddle : EntropyBase {
  EntropyPuddle() : EntropyBase(ENTROPY_PUDDLE_LENGTH) {
  }
};

struct EntropyPuddleWidget : EntropyBaseWidget {
  EntropyPuddleWidget(EntropyPuddle* module) : EntropyBaseWidget(module, "res/EntropyPuddle3.svg") {
    Grid* grid = createWidget<Grid>(mm2px(Vec(4.14, 6.24)));
    grid->setSize(mm2px(Vec(73, 49)));
    grid->module = module;
    // Not from module, as module is null in the preview
    grid->length = ENTROPY_PUDDLE_LENGTH;
    grid->rowLength = ENTROPY_PUDDLE_ROW_LENGTH;
    grid->itemWidth = ENTROPY_PUDDLE_GRID_ITEM_WIDTH;
    addChild(grid);

    float x = 24.64;
    float y = 70;
    float d = 16;
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(x + d * 0, y)), module, EntropyPuddle::START_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(x + d * 1, y)), module, EntropyPuddle::FILTER_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(x + d * 2, y)), module, EntropyPuddle::LENGTH_PARAM));

    x = 7.64;
    y = 82.5;
    d = 11;
    addChild(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(x + d * 0, y)), module, EntropyPuddle::CLOCK_PARAM, EntropyPuddle::CLOCK_LIGHT));
    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(x + d * 1, y)), module, EntropyPuddle::RUN_PARAM, EntropyPuddle::RUN_LIGHT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(x + d * 2, y)), module, EntropyPuddle::START_CV_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(x + d * 3, y)), module, EntropyPuddle::FILTER_CV_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(x + d * 4, y)), module, EntropyPuddle::LENGTH_CV_PARAM));
    addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(x + d * 5, y)), module, EntropyPuddle::RESET_PARAM, EntropyPuddle::RESET_LIGHT));
    addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(x + d * 6, y)), module, EntropyPuddle::RANDOM_PARAM, EntropyPuddle::RANDOM_LIGHT));

    x = 7.64;
    y = 101.114;
    d = 11;
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(x + d * 0, y)), module, EntropyPuddle::CLOCK_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(x + d * 1, y)), module, EntropyPuddle::RUN_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(x + d * 2, y)), module, EntropyPuddle::START_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(x + d * 3, y)), module, EntropyPuddle::FILTER_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(x + d * 4, y)), module, EntropyPuddle::LENGTH_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(x + d * 5, y)), module, EntropyPuddle::RESET_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(x + d * 6, y)), module, EntropyPuddle::RANDOM_INPUT));

    x = 29.64;
    y = 113.115; // Lines up with many VCV plugins
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(x + d * 0, y)), module, EntropyPuddle::EOS_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(x + d * 1, y)), module, EntropyPuddle::TRIGGER_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(x + d * 2, y)), module, EntropyPuddle::CV_OUTPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(x + d * 3, y)), module, EntropyPuddle::SCALE_PARAM));
  }
};

Model* entropyPuddleModel = createModel<EntropyPuddle, EntropyPuddleWidget>("EntropyPuddle");
