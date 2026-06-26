#include "EntropyBase.hpp"

#include <random>

using namespace rack;

EntropyBase::EntropyBase(int length) : length(length) {
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

  configInput(CLOCK_INPUT, "Clock");

  configInput(RUN_INPUT, "Run");
  configParam(RUN_PARAM, 0.f, 1.f, 0.f, "Run");

  configInput(RESET_INPUT, "Reset");
  configParam(RESET_PARAM, 0.f, 1.f, 0.f, "Reset");

  configInput(RANDOM_INPUT, "Randomize");
  configParam(RANDOM_PARAM, 0.f, 1.f, 0.f, "Randomize");

  configInput(START_INPUT, "Start offset");
  configParam(START_PARAM, 0.f, 1.f, 0.f, "Start offset");

  configInput(FILTER_INPUT, "Filter");
  configParam(FILTER_PARAM, 0.f, 1.f, 0.f, "Filter");

  configInput(LENGTH_INPUT, "Sequence length");
  configParam(LENGTH_PARAM, 0.f, 1.f, 1.f, "Sequence length");

  configInput(SCALE_INPUT, "Scale");
  configParam(SCALE_PARAM, 0.f, 1.f, 0.f, "Scale");

  configOutput(CV_OUTPUT, "CV");

  configOutput(TRIGGER_OUTPUT, "Trigger");

  configOutput(EOS_OUTPUT, "End of sequence");

  maxIndex = length - 1;
  randomizeValues();
}

void EntropyBase::onReset() {
  seed = 42u;
  index = 0;
  randomizeValues();
}

void EntropyBase::process(const ProcessArgs& args) {
  updateFilter();
  updateRange();
  updateRun();
  updateValues();
  updateIndex(args);
}

void EntropyBase::updateFilter() {
  minValue = inputs[FILTER_INPUT].isConnected()
    ? inputs[FILTER_INPUT].getVoltage() / 10.f
    : params[FILTER_PARAM].getValue();
}

void EntropyBase::updateRange() {
  float startRatio = inputs[START_INPUT].isConnected()
    ? inputs[START_INPUT].getVoltage() / 10.f
    : params[START_PARAM].getValue();

  minIndex = (int)(clamp01(startRatio) * (float)(length - 0.5f));

  float lengthRatio = inputs[LENGTH_INPUT].isConnected()
    ? inputs[LENGTH_INPUT].getVoltage() / 10.f
    : params[LENGTH_PARAM].getValue();

  maxIndex = (int)(clamp01(lengthRatio) * (float)(length - 0.5f) + minIndex) % length;
}

void EntropyBase::updateRun() {
  if (runInputTrigger.process(inputs[RUN_INPUT].getVoltage())) {
    params[RUN_PARAM].setValue(params[RUN_PARAM].getValue() >= 0.5f ? 0.f : 1.f);
  }

  bool running = params[RUN_PARAM].getValue() >= 0.5f;
  lights[RUN_LIGHT].setBrightness(running ? 1.f : 0.f);
}

void EntropyBase::updateValues() {
  if (
    randomButtonTrigger.process(params[RANDOM_PARAM].getValue()) ||
    randomInputTrigger.process(inputs[RANDOM_INPUT].getVoltage())
  ) {
    std::random_device rd;
    seed = (uint32_t(rd()) << 16) ^ uint32_t(rd());
    randomizeValues();
  }
}

void EntropyBase::updateIndex(const ProcessArgs& args) {
  bool running = params[RUN_PARAM].getValue() >= 0.5f;
  if (running && clockTrigger.process(inputs[CLOCK_INPUT].getVoltage())) {
    clockLightPulse.trigger(0.05f);
    if (index == length - 1) {
      index = 0;
    } else {
      index++;
    }
  }
  lights[CLOCK_LIGHT].setBrightness(clockLightPulse.process(args.sampleTime) ? 1.f : 0.f);

  if (
    !isInRange(index) || // Even if not running or resetting to keep index inside a changing range
    resetButtonTrigger.process(params[RESET_PARAM].getValue()) ||
    resetInputTrigger.process(inputs[RESET_INPUT].getVoltage())
  ) {
    index = minIndex;
  }
}

bool EntropyBase::isInRange(int index) const {
  if (minIndex <= maxIndex) {
    return index >= minIndex && index <= maxIndex;
  } else {
    return index >= minIndex || index <= maxIndex;
  }
}

// TODO: Replace with c++17 clamp(value, 0.f, 1.f)
float EntropyBase::clamp01(float value) {
  if (value < 0.f) {
    return 0.f;
  } else if (value > 1.f) {
    return 1.f;
  } else {
    return value;
  }
}

void EntropyBase::randomizeValues() {
  std::mt19937 rng(seed);
  std::uniform_real_distribution<float> distribution(0.f, 1.f);

  values.clear();
  values.reserve(length);
  for (int i = 0; i < (int)length; ++i) {
    values.push_back(distribution(rng));
  }
}

json_t* EntropyBase::dataToJson() {
  json_t* root = json_object();

  json_t* valuesJson = json_array();
  for (auto& value : values) {
    json_array_append_new(valuesJson, json_real(value));
  }
  json_object_set_new(root, "values", valuesJson);
  json_object_set_new(root, "seed", json_integer(seed));
  json_object_set_new(root, "index", json_integer(index));

  return root;
}

void EntropyBase::dataFromJson(json_t* root) {
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

  if (json_t* currentIndexJson = json_object_get(root, "index")) {
    if (json_is_integer(currentIndexJson)) {
      index = (int)json_integer_value(currentIndexJson);
    }
  }
}
