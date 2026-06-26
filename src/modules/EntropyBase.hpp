#pragma once

#include <rack.hpp>

#include <vector>

struct EntropyBase : rack::Module {
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
    EOS_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightId {
    CLOCK_LIGHT,
    RUN_LIGHT,
    RESET_LIGHT,
    RANDOM_LIGHT,
    NUM_LIGHTS
  };

  std::vector<float> values;
  const int length;
  int minIndex = 0;
  int index = 0;
  int maxIndex = 0; // max is somehat misleading, as it can wrap
  float minValue = 0;

  uint32_t seed = 42u;

  EntropyBase(int length);
  bool isInRange(int index) const;
  void randomizeValues();

private:
  rack::dsp::SchmittTrigger clockTrigger;
  rack::dsp::SchmittTrigger runInputTrigger;
  rack::dsp::SchmittTrigger resetButtonTrigger;
  rack::dsp::SchmittTrigger resetInputTrigger;
  rack::dsp::SchmittTrigger randomButtonTrigger;
  rack::dsp::SchmittTrigger randomInputTrigger;
  rack::dsp::PulseGenerator clockLightPulse;

  void onReset() override;

  void process(const ProcessArgs& args) override;
  void updateFilter();
  void updateRange();
  void updateRun();
  void updateValues();
  void updateIndex(const ProcessArgs& args);

  static float clamp01(float value);
  json_t* dataToJson() override;
  void dataFromJson(json_t* root) override;
};
