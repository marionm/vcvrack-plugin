#pragma once

#include <rack.hpp>

#include <vector>

struct EntropyBase : rack::Module {
  std::vector<float> values;
  const int length;
  int minIndex = 0;
  int index = 0;
  int maxIndex = 0; // max is somehat misleading, as it can wrap

  struct ParamIds {
    int run, reset, random, start, length, filter;
  };

  struct InputIds {
    int clock, run, reset, random, start, length, filter;
  };

  struct OutputIds {
    int cv, trigger, endOfSequence;
  };

  struct LightIds {
    int clock, run;
  };

  struct Ids {
    ParamIds params;
    InputIds inputs;
    OutputIds outputs;
    LightIds lights;
  };

  uint32_t seed = 42u;

  EntropyBase(int length);
  void configPorts(const Ids& ids);
  bool isInRange(int index) const;
  void randomizeValues();

protected:
  Ids ids;

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
  void updateRange();
  void incrementIndex();
  static float clamp01(float value);
  json_t* dataToJson() override;
  void dataFromJson(json_t* root) override;
};
