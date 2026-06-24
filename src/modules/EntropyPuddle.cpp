#include "EntropyPuddle.hpp"

using namespace rack;

EntropyPuddle::EntropyPuddle() : EntropyBase(ENTROPY_PUDDLE_LENGTH) {
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

  configPorts({
    .params  = { RUN_PARAM, RESET_PARAM, RANDOM_PARAM, START_PARAM, LENGTH_PARAM, FILTER_PARAM },
    .inputs  = { CLOCK_INPUT, RUN_INPUT, RESET_INPUT, RANDOM_INPUT, START_INPUT, LENGTH_INPUT, FILTER_INPUT },
    .outputs = { CV_OUTPUT, TRIGGER_OUTPUT, EOS_OUTPUT },
    .lights  = { CLOCK_LIGHT, RUN_LIGHT }
  });
}
