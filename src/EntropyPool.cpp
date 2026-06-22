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
  int currentIndex = 0;
  int startIndex = 0;
  int endIndex = ENTROPY_POOL_VALUES_SIZE - 1;
  std::vector<float> values;
  uint32_t seed = 42u;
  dsp::SchmittTrigger clockTrigger;
  dsp::SchmittTrigger runInputTrigger;
  dsp::SchmittTrigger resetButtonTrigger;
  dsp::SchmittTrigger resetInputTrigger;
  dsp::SchmittTrigger randomButtonTrigger;
  dsp::SchmittTrigger randomInputTrigger;
  dsp::PulseGenerator clockLightPulse;

  EntropyPool() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    configParam(RUN_PARAM, 0.f, 1.f, 0.f, "Run");
    configParam(RESET_PARAM, 0.f, 1.f, 0.f, "Reset");
    configParam(RANDOM_PARAM, 0.f, 1.f, 0.f, "Randomize");
    configParam(START_PARAM, 0.f, 1.f, 0.f, "Start offset");
    configParam(LENGTH_PARAM, 0.f, 1.f, 1.f, "Sequence length");
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
    randomizeValues();
  }

  void onReset() override {
    seed = 42u;
    currentIndex = 0;
    randomizeValues();
  }

  void process(const ProcessArgs& args) override {
    if (runInputTrigger.process(inputs[RUN_INPUT].getVoltage(), 0.1f, 1.f)) {
      params[RUN_PARAM].setValue(params[RUN_PARAM].getValue() >= 0.5f ? 0.f : 1.f);
    }

    bool running = params[RUN_PARAM].getValue() >= 0.5f;
    lights[RUN_LIGHT].setBrightness(running ? 1.f : 0.f);

    updateRange();
    int size = sequenceSize();

    if (
      resetButtonTrigger.process(params[RESET_PARAM].getValue(), 0.1f, 1.f) ||
      resetInputTrigger.process(inputs[RESET_INPUT].getVoltage(), 0.1f, 1.f)
    ) {
      currentIndex = 0;
    }

    if (running && clockTrigger.process(inputs[CLOCK_INPUT].getVoltage(), 0.1f, 1.f)) {
      currentIndex = nextIndexInRange(currentIndex, startIndex, endIndex, size);
      clockLightPulse.trigger(0.05f);
    }

    if (!isInRange(currentIndex, startIndex, endIndex, size)) {
      currentIndex = startIndex;
    }

    lights[CLOCK_LIGHT].setBrightness(clockLightPulse.process(args.sampleTime) ? 1.f : 0.f);

    if (
      randomButtonTrigger.process(params[RANDOM_PARAM].getValue(), 0.1f, 1.f) ||
      randomInputTrigger.process(inputs[RANDOM_INPUT].getVoltage(), 0.1f, 1.f)
    ) {
      std::random_device rd;
      seed = (uint32_t(rd()) << 16) ^ uint32_t(rd());
      randomizeValues();
    }
  }

  int sequenceSize() const {
    return values.empty() ? valuesSize : (int)values.size();
  }

  static float clamp01(float value) {
    if (value < 0.f) return 0.f;
    if (value > 1.f) return 1.f;
    return value;
  }

  int normalizedToIndex(float normalized, int size) const {
    if (size <= 1) {
      return 0;
    }
    normalized = clamp01(normalized);
    return (int)(normalized * (float)(size - 1) + 0.5f);
  }

  int normalizedToLength(float normalized, int size) const {
    if (size <= 1) {
      return 1;
    }
    normalized = clamp01(normalized);
    return 1 + (int)(normalized * (float)(size - 1) + 0.5f);
  }

  bool isInRange(int index, int startIndex, int endIndex, int size) const {
    if (size <= 0) {
      return false;
    }
    if (startIndex <= endIndex) {
      return index >= startIndex && index <= endIndex;
    }
    return index >= startIndex || index <= endIndex;
  }

  void updateRange() {
    int size = sequenceSize();
    float startValue = inputs[START_INPUT].isConnected()
      ? inputs[START_INPUT].getVoltage() / 10.f
      : params[START_PARAM].getValue();
    float lengthValue = inputs[LENGTH_INPUT].isConnected()
      ? inputs[LENGTH_INPUT].getVoltage() / 10.f
      : params[LENGTH_PARAM].getValue();

    startIndex = normalizedToIndex(startValue, size);
    int length = normalizedToLength(lengthValue, size);
    endIndex = (startIndex + length - 1) % size;
  }

  bool isInActiveRange(int index) const {
    return isInRange(index, startIndex, endIndex, sequenceSize());
  }

  int nextIndexInRange(int index, int startIndex, int endIndex, int size) const {
    if (size <= 0) {
      return 0;
    }

    if (!isInRange(index, startIndex, endIndex, size)) {
      return startIndex;
    }

    index = (index + 1) % size;
    while (!isInRange(index, startIndex, endIndex, size)) {
      index = (index + 1) % size;
    }
    return index;
  }

  void randomizeValues() {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> distribution(0.f, 1.f);

    values.clear();
    values.reserve(valuesSize);
    for (int i = 0; i < (int)valuesSize; ++i) {
      values.push_back(distribution(rng));
    }

    currentIndex %= sequenceSize();
  }

  json_t* dataToJson() override {
    json_t* root = json_object();

    json_t* valuesJson = json_array();
    for (auto& value : values) {
      json_array_append_new(valuesJson, json_real(value));
    }
    json_object_set_new(root, "values", valuesJson);
    json_object_set_new(root, "seed", json_integer(seed));
    json_object_set_new(root, "currentIndex", json_integer(currentIndex));

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

    if (json_t* currentIndexJson = json_object_get(root, "currentIndex")) {
      if (json_is_integer(currentIndexJson)) {
        currentIndex = (int)json_integer_value(currentIndexJson);
      }
    }

    int size = sequenceSize();
    if (size > 0) {
      currentIndex %= size;
      if (currentIndex < 0) {
        currentIndex += size;
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
      const int size = module ? module->sequenceSize() : ENTROPY_POOL_VALUES_SIZE;
      for (int i = 0; i < size; i++) {
        int x = 1 + (i % 24) * (width + 1);
        int y = 1 + (i / 24) * (width + 1);
        float value = module && i < (int)module->values.size() ? module->values[i] : dist(rng);

        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, mm2px(x), mm2px(y), mm2px(width), mm2px(width), 3.f);

        // NVGcolor color = nvgRGBA(86, 211, 100, value * 255.f);
        NVGcolor color = nvgRGBA(46, 160, 67, value * 255.f);
        nvgFillColor(args.vg, color);
        nvgFill(args.vg);

        bool inRange = module ? module->isInActiveRange(i) : false;
        if (inRange) {
          nvgStrokeWidth(args.vg, mm2px(0.35));
          nvgStrokeColor(args.vg, nvgRGBA(255, 255, 255, 60));
          nvgStroke(args.vg);
        }

        if (i == (module ? module->currentIndex : 0)) {
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
