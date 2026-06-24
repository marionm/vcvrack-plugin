#include "EntropyBase.hpp"

static constexpr int ENTROPY_PUDDLE_LENGTH = 100;

struct EntropyPuddle : EntropyBase {
  enum ParamId {
    RUN_PARAM,
    RESET_PARAM,
    RANDOM_PARAM,
    START_PARAM,
    LENGTH_PARAM,
    FILTER_PARAM,
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

  EntropyPuddle();
};
