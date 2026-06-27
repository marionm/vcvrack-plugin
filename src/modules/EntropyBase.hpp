#pragma once

#include <rack.hpp>

#include <vector>

struct EntropyBase : rack::Module {
  enum ParamId {
    CLOCK_PARAM,
    RUN_PARAM,
    RESET_PARAM,
    RANDOM_PARAM,
    SCALE_PARAM,
    START_PARAM,
    START_CV_PARAM,
    LENGTH_PARAM,
    LENGTH_CV_PARAM,
    FILTER_PARAM,
    FILTER_CV_PARAM,
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
    NUM_INPUTS
  };

  enum OutputId {
    EOS_OUTPUT,
    TRIGGER_OUTPUT,
    CV_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightId {
    CLOCK_LIGHT,
    RUN_LIGHT,
    RESET_LIGHT,
    RANDOM_LIGHT,
    EOS_LIGHT,
    TRIGGER_LIGHT,
    NUM_LIGHTS
  };

  std::vector<float> values;
  const int totalLength;
  int minIndex = 0;
  int index = 0;
  int maxIndex = 0; // max is somehat misleading, as it can wrap
  float minValue = 0;
  float maxValue = 0;
  float gridItemWidth = 5;

  uint32_t seed = 42u;

  EntropyBase(int totalLength);
  bool isInRange(int index) const;
  void randomizeSeed();
  void randomizeValues();

private:
  rack::dsp::SchmittTrigger clockButtonTrigger;
  rack::dsp::SchmittTrigger clockTrigger;
  rack::dsp::PulseGenerator clockPulse;

  rack::dsp::SchmittTrigger runTrigger;

  rack::dsp::SchmittTrigger resetButtonTrigger;
  rack::dsp::SchmittTrigger resetTrigger;
  rack::dsp::PulseGenerator resetPulse;

  rack::dsp::SchmittTrigger randomButtonTrigger;
  rack::dsp::SchmittTrigger randomTrigger;
  rack::dsp::PulseGenerator randomPulse;

  rack::dsp::PulseGenerator eosPulse;
  rack::dsp::PulseGenerator triggerPulse;

  void onRandomize() override;
  void onReset() override;

  void process(const ProcessArgs& args) override;
  void updateFilter();
  void updateRun();
  void updateValues(const ProcessArgs& args);
  bool updateRange();
  void updateIndex(const ProcessArgs& args, bool isReversed);
  void pulseLight(const ProcessArgs& args, rack::dsp::PulseGenerator& pulse, int lightId, bool on);

  static float clamp01(float value);
  static float clamp11(float value);
  bool clampIndex(bool isReversed);
  int clampRangeIndex(int index);

  json_t* dataToJson() override;
  void dataFromJson(json_t* root) override;
};
