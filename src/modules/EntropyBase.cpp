#include "EntropyBase.hpp"

#include <random>

using namespace rack;

EntropyBase::EntropyBase(int length) : length(length) {
  maxIndex = length - 1;
  randomizeValues();
}

void EntropyBase::configPorts(const Ids& ids) {
  this->ids = ids;

  configParam(ids.params.run, 0.f, 1.f, 0.f, "Run");
  configParam(ids.params.reset, 0.f, 1.f, 0.f, "Reset");
  configParam(ids.params.random, 0.f, 1.f, 0.f, "Randomize");
  configParam(ids.params.start, 0.f, 1.f, 0.f, "Start offset");
  configParam(ids.params.length, 0.f, 1.f, 1.f, "Sequence length");
  configParam(ids.params.filter, 0.f, 1.f, 0.f, "Filter");

  configInput(ids.inputs.clock, "Clock");
  configInput(ids.inputs.run, "Run");
  configInput(ids.inputs.reset, "Reset");
  configInput(ids.inputs.random, "Randomize");
  configInput(ids.inputs.start, "Start offset");
  configInput(ids.inputs.length, "Sequence length");
  configInput(ids.inputs.filter, "Filter");

  configOutput(ids.outputs.cv, "CV");
  configOutput(ids.outputs.trigger, "Trigger");
  configOutput(ids.outputs.endOfSequence, "End of sequence");
}

void EntropyBase::onReset() {
  seed = 42u;
  index = 0;
  randomizeValues();
}

void EntropyBase::process(const ProcessArgs& args) {
  if (runInputTrigger.process(inputs[ids.inputs.run].getVoltage())) {
    params[ids.params.run].setValue(params[ids.params.run].getValue() >= 0.5f ? 0.f : 1.f);
  }

  bool running = params[ids.params.run].getValue() >= 0.5f;
  lights[ids.lights.run].setBrightness(running ? 1.f : 0.f);

  updateRange();

  if (running && clockTrigger.process(inputs[ids.inputs.clock].getVoltage())) {
    incrementIndex();
    clockLightPulse.trigger(0.05f);
  }
  lights[ids.lights.clock].setBrightness(clockLightPulse.process(args.sampleTime) ? 1.f : 0.f);

  if (
    !isInRange(index) || // Even if not running or resetting to keep index inside a changing range
    resetButtonTrigger.process(params[ids.params.reset].getValue()) ||
    resetInputTrigger.process(inputs[ids.inputs.reset].getVoltage())
  ) {
    index = minIndex;
  }

  if (
    randomButtonTrigger.process(params[ids.params.random].getValue()) ||
    randomInputTrigger.process(inputs[ids.inputs.random].getVoltage())
  ) {
    std::random_device rd;
    seed = (uint32_t(rd()) << 16) ^ uint32_t(rd());
    randomizeValues();
  }
}

void EntropyBase::updateRange() {
  float startRatio = inputs[ids.inputs.start].isConnected()
    ? inputs[ids.inputs.start].getVoltage() / 10.f
    : params[ids.params.start].getValue();

  minIndex = (int)(clamp01(startRatio) * (float)(length - 0.5f));

  float lengthRatio = inputs[ids.inputs.length].isConnected()
    ? inputs[ids.inputs.length].getVoltage() / 10.f
    : params[ids.params.length].getValue();

  maxIndex = (int)(clamp01(lengthRatio) * (float)(length - 0.5f) + minIndex) % length;
}

bool EntropyBase::isInRange(int index) const {
  if (minIndex <= maxIndex) {
    return index >= minIndex && index <= maxIndex;
  } else {
    return index >= minIndex || index <= maxIndex;
  }
}

void EntropyBase::incrementIndex() {
  if (index == length - 1) {
    index = 0;
  } else {
    index++;
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
