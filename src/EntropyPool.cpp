#include "plugin.hpp"
#include "IntegrationsModal.hpp"
#include "SeedModal.hpp"
#include <random>

using namespace rack;

static constexpr int ENTROPY_POOL_VALUES_SIZE = 240;

struct EntropyPool : Module {
  enum ParamId {
    RUN_PARAM,
    RESET_PARAM,
    RANDOM_PARAM,
    START_PARAM,
    LENGTH_PARAM,
    FILTER_PARAM,
    SCALE_PARAM,
    NUM_PARAMS
  };

  enum InputId {
    CLOCK_INPUT,
    RUN_INPUT,
    RESET_INPUT,
    RANDOM_INPUT,
    START_INPUT,
    LENGTH_INPUT,
    FILTER_INPUT,
    SCALE_INPUT,
    NUM_INPUTS
  };

  enum OutputId {
    CV_OUTPUT,
    TRIGGER_OUTPUT,
    END_OF_SEQUENCE_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightId {
    CLOCK_LIGHT,
    RUN_LIGHT,
    RESET_LIGHT,
    RANDOM_LIGHT,
    NUM_LIGHTS
  };

  int valuesSize = ENTROPY_POOL_VALUES_SIZE;
  std::vector<float> values;
  uint32_t seed = 0;
  dsp::SchmittTrigger randomButtonTrigger;
  dsp::SchmittTrigger randomInputTrigger;

  EntropyPool() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    configParam(RUN_PARAM, 0.f, 1.f, 0.f, "Run");
    configParam(RESET_PARAM, 0.f, 1.f, 0.f, "Reset");
    configParam(RANDOM_PARAM, 0.f, 1.f, 0.f, "Randomize");
    configParam(START_PARAM, 0.f, 1.f, 0.f, "Start offset");
    configParam(LENGTH_PARAM, 0.f, 1.f, 0.f, "Sequence length");
    configParam(FILTER_PARAM, 0.f, 1.f, 0.f, "Filter");
    configParam(SCALE_PARAM, 0.f, 1.f, 0.f, "Scale");

    configInput(CLOCK_INPUT, "Clock");
    configInput(RUN_INPUT, "Run");
    configInput(RESET_INPUT, "Reset");
    configInput(RANDOM_INPUT, "Randomize");
    configInput(START_INPUT, "Start offset");
    configInput(LENGTH_INPUT, "Sequence length");
    configInput(FILTER_INPUT, "Filter");
    configInput(SCALE_INPUT, "Scale");

    configOutput(CV_OUTPUT, "CV");
    configOutput(TRIGGER_OUTPUT, "Trigger");
    configOutput(END_OF_SEQUENCE_OUTPUT, "End of sequence");

  }

  void process(const ProcessArgs&) override {
    if (
      randomButtonTrigger.process(params[RANDOM_PARAM].getValue(), 0.1f, 1.f) ||
      randomInputTrigger.process(inputs[RANDOM_INPUT].getVoltage(), 0.1f, 1.f)
    ) {
      std::random_device rd;
      seed = (uint32_t(rd()) << 16) ^ uint32_t(rd());
      randomizeValues();
    }
  }

  void randomizeValues() {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> distribution(0.f, 1.f);

    values.clear();
    values.reserve(valuesSize);
    for (int i = 0; i < (int)valuesSize; ++i) {
      values.push_back(distribution(rng));
    }
  }

  json_t* dataToJson() override {
    json_t* root = json_object();

    json_t* valuesJson = json_array();
    for (auto& value : values) {
      json_array_append_new(valuesJson, json_real(value));
    }
    json_object_set_new(root, "values", valuesJson);
    json_object_set_new(root, "seed", json_integer(seed));

    return root;
  }

  void dataFromJson(json_t* root) override {
    if (json_t* valuesJson = json_object_get(root, "values")) {
      if (json_is_array(valuesJson)) {
        values.clear();
        size_t index;
        json_t* valueJson;
        json_array_foreach(valuesJson, index, valueJson) {
          if (json_is_number(valueJson)) {
            values.push_back((float)json_number_value(valueJson));
          }
        }
      }
    }

    if (json_t* seedJson = json_object_get(root, "seed")) {
      if (json_is_integer(seedJson)) {
        seed = (uint32_t)json_integer_value(seedJson);
      }
    }
  }
};

// TODO: Parameterize and extract
// TODO: Render tooltips by calculating index from mouse position
struct Grid : Widget {
  EntropyPool* module = nullptr;
  int width = 5;
  std::mt19937 rng{42u};
  std::uniform_real_distribution<float> dist{0.f, 1.f};

  void draw(const DrawArgs &args) override {
    try {
      const int size = module ? (int)module->values.size() : ENTROPY_POOL_VALUES_SIZE;
      for (int i = 0; i < size; i++) {
        int x = 1 + (i % 24) * (width + 1);
        int y = 1 + (i / 24) * (width + 1);
        float value = module ? module->values[i] : dist(rng);

        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, mm2px(x), mm2px(y), mm2px(width), mm2px(width), 3.f);

        // NVGcolor color = nvgRGBA(86, 211, 100, value * 255.f);
        NVGcolor color = nvgRGBA(46, 160, 67, value * 255.f);
        nvgFillColor(args.vg, color);

        nvgFill(args.vg);

        if (i == 42) {
          nvgStrokeWidth(args.vg, mm2px(0.5));
          nvgStrokeColor(args.vg, nvgRGB(255, 255, 255));
          nvgStroke(args.vg);
        }
      }
    } catch (...) {
      DEBUG("error in Grid::draw");
    }
  }
};

struct EntropyPoolWidget : app::ModuleWidget {
  EntropyPoolWidget(EntropyPool* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/EntropyPool.svg")));

    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    Grid* grid = createWidget<Grid>(mm2px(Vec(6.24, 21.5)));
    grid->module = module;
    grid->setSize(mm2px(Vec(145, 61)));
    addChild(grid);

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
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(outputsX + outputsDelta * 2, outputsY)), module, EntropyPool::END_OF_SEQUENCE_OUTPUT));
  }

  void appendContextMenu(ui::Menu* menu) override {
    ModuleWidget::appendContextMenu(menu);

    EntropyPool* m = getModule<EntropyPool>();
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
      IntegrationsModal* modal = new IntegrationsModal(m->valuesSize, [m](const std::vector<float>& values) {
        m->values = values;
      });
      APP->scene->addChild(modal);
    }));
  }

};

Model* modelEntropyPool = createModel<EntropyPool, EntropyPoolWidget>("EntropyPool");
