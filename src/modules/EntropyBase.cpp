#include "EntropyBase.hpp"
#include "../components/FilterParamQuantity.hpp"
#include "../components/LengthParamQuantity.hpp"
#include "../components/StartParamQuantity.hpp"

#include <random>
#include <string>

using namespace rack;

EntropyBase::EntropyBase(int totalLength) : totalLength(totalLength) {
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

  std::string clockLabel = "Clock";
  configInput(CLOCK_INPUT, clockLabel);
  configButton(CLOCK_PARAM, clockLabel);

  std::string runLabel = "Run";
  configInput(RUN_INPUT, runLabel);
  configButton(RUN_PARAM, runLabel);

  std::string resetLabel = "Reset";
  configInput(RESET_INPUT, resetLabel);
  configButton(RESET_PARAM, resetLabel);

  std::string randomLabel = "Randomize";
  configInput(RANDOM_INPUT, randomLabel);
  configButton(RANDOM_PARAM, randomLabel);

  std::string startLabel = "Start index";
  configInput(START_INPUT, startLabel);
  configParam<StartParamQuantity>(START_PARAM, 0.f, 1.f, 0.f, startLabel);
  configParam(START_CV_PARAM, -1.f, 1.f, 0.f, startLabel.append(" CV"), "%", 0, 100);
  getParamQuantity(START_CV_PARAM)->randomizeEnabled = false;

  std::string filterLabel = "Filter";
  configInput(FILTER_INPUT, filterLabel);
  configParam<FilterParamQuantity>(FILTER_PARAM, -1.f, 1.f, 0.f, filterLabel);
  configParam(FILTER_CV_PARAM, -1.f, 1.f, 0.f, filterLabel.append(" CV"), "%", 0, 100);
  getParamQuantity(FILTER_CV_PARAM)->randomizeEnabled = false;

  int initialLength = 8;
  std::string lengthLabel = "Length";
  configInput(LENGTH_INPUT, lengthLabel);
  configParam<LengthParamQuantity>(LENGTH_PARAM, -1.f, 1.f, (float)initialLength / totalLength, lengthLabel);
  configParam(LENGTH_CV_PARAM, -1.f, 1.f, 0.f, lengthLabel.append(" CV"), "%", 0, 100);
  getParamQuantity(LENGTH_CV_PARAM)->randomizeEnabled = false;

  configOutput(EOS_OUTPUT, "End of sequence");

  configOutput(TRIGGER_OUTPUT, "Trigger");

  configOutput(CV_OUTPUT, "CV");

  configParam(SCALE_PARAM, -1.f, 1.f, .1f, "Scale");
  getParamQuantity(SCALE_PARAM)->randomizeEnabled = false;

  maxIndex = totalLength - 1;
  ((LengthParamQuantity*)getParamQuantity(LENGTH_PARAM))->totalLength = totalLength;
  ((StartParamQuantity*)getParamQuantity(START_PARAM))->totalLength = totalLength;

  randomizeValues();
}

void EntropyBase::onRandomize() {
  Module::onRandomize();
  randomizeSeed();
  randomizeValues();
}

void EntropyBase::onReset() {
  seed = 42u;
  index = 0;
  randomizeValues();
}

void EntropyBase::process(const ProcessArgs& args) {
  updateFilter();
  updateRun();
  updateValues(args);

  bool isReversed = updateRange();
  updateIndex(args, isReversed);
}

void EntropyBase::updateFilter() {
  float filter = inputs[FILTER_INPUT].isConnected()
    ? inputs[FILTER_INPUT].getVoltage() / 10.f
    : params[FILTER_PARAM].getValue();

  if (filter == 0.f) {
    minValue = 0.f;
    maxValue = 1.f;
  } else if (filter > 0.f) {
    minValue = filter;
    maxValue = 1.f;
  } else {
    minValue = 0.f;
    maxValue = 1 + filter;
  }
}

bool EntropyBase::updateRange() {
  float startRatio = inputs[START_INPUT].isConnected()
    ? inputs[START_INPUT].getVoltage() / 10.f
    : params[START_PARAM].getValue();

  int startIndex = (int)(clamp01(startRatio) * (float)(totalLength - 0.5f));

  float lengthRatio = inputs[LENGTH_INPUT].isConnected()
    ? inputs[LENGTH_INPUT].getVoltage() / 10.f
    : params[LENGTH_PARAM].getValue();

  int length = (int)(clamp11(lengthRatio) * (float)(totalLength - 0.5f));
  bool isReversed = length < 0;
  if (isReversed) {
    minIndex = clampRangeIndex(startIndex + length);
    maxIndex = clampRangeIndex(startIndex);
    ((StartParamQuantity*)getParamQuantity(START_PARAM))->index = maxIndex;
    ((LengthParamQuantity*)getParamQuantity(LENGTH_PARAM))->length = length - 1;
  } else {
    minIndex = clampRangeIndex(startIndex);
    maxIndex = clampRangeIndex(startIndex + length);
    ((StartParamQuantity*)getParamQuantity(START_PARAM))->index = minIndex;
    ((LengthParamQuantity*)getParamQuantity(LENGTH_PARAM))->length = length + 1;
  }

  clampIndex(isReversed);

  return isReversed;
}

void EntropyBase::updateRun() {
  if (runTrigger.process(inputs[RUN_INPUT].getVoltage())) {
    params[RUN_PARAM].setValue(params[RUN_PARAM].getValue() >= 0.5f ? 0.f : 1.f);
  }

  bool running = params[RUN_PARAM].getValue() >= 0.5f;
  lights[RUN_LIGHT].setBrightness(running ? 1.f : 0.f);
}

void EntropyBase::updateValues(const ProcessArgs& args) {
  bool didRandomize = false;
  if (
    randomButtonTrigger.process(params[RANDOM_PARAM].getValue()) ||
    randomTrigger.process(inputs[RANDOM_INPUT].getVoltage())
  ) {
    randomizeSeed();
    randomizeValues();
    didRandomize = true;
  }

  pulseLight(args, randomPulse, RANDOM_LIGHT, didRandomize);
}

void EntropyBase::updateIndex(const ProcessArgs& args, bool isReversed) {
  bool didStep = false;
  bool hitEos = false;
  bool running = params[RUN_PARAM].getValue() >= 0.5f;
  if (running && (
    clockButtonTrigger.process(params[CLOCK_PARAM].getValue()) ||
    clockTrigger.process(inputs[CLOCK_INPUT].getVoltage())
  )) {
    index += isReversed ? -1 : 1;
    didStep = true;
    hitEos = clampIndex(isReversed);
  }

  pulseLight(args, clockPulse, CLOCK_LIGHT, didStep);
  pulseLight(args, eosPulse, EOS_LIGHT, hitEos);

  bool didReset = false;
  if (
    resetButtonTrigger.process(params[RESET_PARAM].getValue()) ||
    resetTrigger.process(inputs[RESET_INPUT].getVoltage())
  ) {
    index = isReversed ? maxIndex : minIndex;
    didReset = true;
  }

  pulseLight(args, resetPulse, RESET_LIGHT, didReset);
}

void EntropyBase::pulseLight(const ProcessArgs& args, dsp::PulseGenerator& pulse, int lightId, bool on) {
  if (on) {
    pulse.trigger(1e-3f);
    pulse.process(args.sampleTime);
  }

  lights[lightId].setSmoothBrightness(on, args.sampleTime);
}

bool EntropyBase::isInRange(int index) const {
  if (minIndex <= maxIndex) {
    return index >= minIndex && index <= maxIndex;
  } else {
    return index >= minIndex || index <= maxIndex;
  }
}

// TODO: Replace clamp helpers with c++17 clamp
float EntropyBase::clamp01(float value) {
  if (value < 0.f) {
    return 0.f;
  } else if (value > 1.f) {
    return 1.f;
  } else {
    return value;
  }
}

float EntropyBase::clamp11(float value) {
  if (value < -1.f) {
    return -1.f;
  } else if (value > 1.f) {
    return 1.f;
  } else {
    return value;
  }
}

bool EntropyBase::clampIndex(bool isReversed) {
  if (!isInRange(index)) {
    index = isReversed ? maxIndex : minIndex;
    return true;
  } else {
    return false;
  }
}

int EntropyBase::clampRangeIndex(int index) {
  if (index < 0) {
    return index + totalLength;
  } else if (index >= totalLength) {
    return index - totalLength;
  } else {
    return index;
  }
}

void EntropyBase::randomizeSeed() {
  std::random_device rd;
  seed = (uint32_t(rd()) << 16) ^ uint32_t(rd());
}

void EntropyBase::randomizeValues() {
  std::mt19937 rng(seed);
  std::uniform_real_distribution<float> distribution(0.f, 1.f);

  values.clear();
  values.reserve(totalLength);
  for (int i = 0; i < (int)totalLength; ++i) {
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
