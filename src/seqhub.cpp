#include "plugin.hpp"
#include "IntegrationsModal.hpp"
#include <sstream>

using namespace rack;

// Sequencer driven by Github contribution activity
struct Seqhub : Module {
  enum ParamId {
    RUN_PARAM,
    RESET_PARAM,
    RANDOM_PARAM,
    START_DATE_PARAM,
    LENGTH_PARAM,
    PING_PONG_PARAM,
    WORKING_WEEKENDS_PARAM,
    RISE_SPEED_PARAM,
    FALL_SPEED_PARAM,
    FILTER_PARAM,
    SCALE_PARAM,
    NUM_PARAMS
  };

  enum InputId {
    CLOCK_INPUT,
    RUN_INPUT,
    RESET_INPUT,
    RANDOM_INPUT,
    START_DATE_INPUT,
    LENGTH_INPUT,
    PING_PONG_INPUT,
    WORKING_WEEKENDS_INPUT,
    RISE_SPEED_INPUT,
    FALL_SPEED_INPUT,
    FILTER_INPUT,
    SCALE_INPUT,
    NUM_INPUTS
  };

  enum OutputId {
    CV_OUTPUT,
    GATE_OUTPUT,
    TRIGGER_OUTPUT,
    END_OF_SEQUENCE_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightId {
    CLOCK_LIGHT,
    RUN_LIGHT,
    RESET_LIGHT,
    RANDOM_LIGHT,
    PING_PONG_LIGHT,
    WORKING_WEEKENDS_LIGHT,
    NUM_LIGHTS
  };

  std::vector<float> contributionsPerDay;
  std::string startDate;

  Seqhub() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    configParam(RUN_PARAM, 0.f, 1.f, 0.f, "Run");
    configParam(RESET_PARAM, 0.f, 1.f, 0.f, "Reset");
    configParam(RANDOM_PARAM, 0.f, 1.f, 0.f, "Randomize");
    configParam(START_DATE_PARAM, 0.f, 1.f, 0.f, "Start date");
    configParam(LENGTH_PARAM, 0.f, 1.f, 0.f, "Sequence length");
    configParam(PING_PONG_PARAM, 0.f, 1.f, 0.f, "Ping pong");
    configParam(WORKING_WEEKENDS_PARAM, 0.f, 1.f, 0.f, "Working weekends");
    configParam(RISE_SPEED_PARAM, 0.f, 1.f, 0.f, "Rise speed");
    configParam(FALL_SPEED_PARAM, 0.f, 1.f, 0.f, "Fall speed");
    configParam(FILTER_PARAM, 0.f, 1.f, 0.f, "Filter");
    configParam(SCALE_PARAM, 0.f, 1.f, 0.f, "Scale");

    configInput(CLOCK_INPUT, "Clock");
    configInput(RUN_INPUT, "Run");
    configInput(RESET_INPUT, "Reset");
    configInput(RANDOM_INPUT, "Randomize");
    configInput(START_DATE_INPUT, "Start date");
    configInput(LENGTH_INPUT, "Sequence length");
    configInput(PING_PONG_INPUT, "Ping pong");
    configInput(WORKING_WEEKENDS_INPUT, "Working weekends");
    configInput(RISE_SPEED_INPUT, "Rise speed");
    configInput(FALL_SPEED_INPUT, "Fall speed");
    configInput(FILTER_INPUT, "Filter");
    configInput(SCALE_INPUT, "Scale");

    configOutput(CV_OUTPUT, "CV");
    configOutput(GATE_OUTPUT, "Gate");
    configOutput(TRIGGER_OUTPUT, "Trigger");
    configOutput(END_OF_SEQUENCE_OUTPUT, "End of sequence");

  }

  ~Seqhub() override {
  }

  void process(const ProcessArgs&) override {
  }

  void setContributionsPerDay(std::vector<float> values) {
    this->contributionsPerDay = std::move(values);
  }

  json_t* dataToJson() override {
    json_t* root = json_object();

    json_object_set_new(root, "startDate", json_string(startDate.c_str()));

    json_t* contributionsPerDayJson = json_array();
    for (auto& contributions : contributionsPerDay) {
      json_array_append_new(contributionsPerDayJson, json_integer(contributions));
    }
    json_object_set_new(root, "contributionsPerDay", contributionsPerDayJson);

    return root;
  }

  void dataFromJson(json_t* root) override {
    if (auto startDateJson = json_object_get(root, "startDate")) {
      startDate = json_string_value(startDateJson);
    }

    if (json_t* contributionsPerDayJson = json_object_get(root, "contributionsPerDay")) {
      if (json_is_array(contributionsPerDayJson)) {
        contributionsPerDay.clear();
        size_t index;
        json_t* contributionsJson;
        json_array_foreach(contributionsPerDayJson, index, contributionsJson) {
          if (json_is_integer(contributionsJson)) {
            contributionsPerDay.push_back(json_integer_value(contributionsJson));
          }
        }
      }
    }

    logState("dataFromJson");
  }

  void logState(std::string prefix) {
    std::ostringstream oss;
    for (size_t i = 0; i < contributionsPerDay.size(); ++i) {
      oss << contributionsPerDay[i] << ", ";
    }

    DEBUG("%s", prefix.c_str());
    DEBUG("contributionsPerDay: %s", oss.str().c_str());
    DEBUG("startDate: %s", startDate.c_str());
  }
};

struct Contributions : Widget {
  Seqhub* module = nullptr;

  void draw(const DrawArgs &args) override {
    if (!module) {
      // TODO: Draw random sample
      return;
    }

    try {
      DEBUG("size: %i", (int)module->contributionsPerDay.size());
      // TODO: 360 in a 36x10 grid is nice... but is weird with weeks
      //   Is there a width i can make work that is divisible by 7? (Or tall and render vertically?)
      //   i guess 35x10 could be ok... can reduce overall width of the module by a bit if it looks weird
      //   matters less with truly random data... could also consider an "alignment" toggle or something
      for (int i = 0; i < (int)module->contributionsPerDay.size(); i++) {
      // for (int i = 0; i < 360; i++) {
      // for (int i = 0; i < 350; i++) {
        int x = 4 + (i % 35) * 5;
        int y = 4 + (i / 35) * 5;

        // NVGcolor color = nvgRGBA(0, 255, 0, rand() * 200 + 55);

        // TODO: This color formula is not great:
        //   Github actually uses multiple, quantized colors
        //   I do need a gradient, but maybe do so with clear cutoffs like them?
        //   Or, at least pick the right one from their set that works well with alpha
        //   Also, feels off w/scaling? non-zero but small values are not visible
        int value = module->contributionsPerDay[i];
        NVGcolor color = nvgRGBA(25, 108, 46, value / 10.f * 255.f);

        nvgBeginPath(args.vg);
        nvgRect(args.vg, mm2px(x), mm2px(y), mm2px(4), mm2px(4));
        nvgFillColor(args.vg, color);
        nvgFill(args.vg);

        if (i == 42) {
          nvgStrokeWidth(args.vg, mm2px(0.5));
          nvgStrokeColor(args.vg, nvgRGB(255, 255, 255));
          nvgStroke(args.vg);
        }
      }
    } catch (...) {
      DEBUG("error in Contributions::draw");
    }
  }
};

struct SeqhubWidget : app::ModuleWidget {
  SeqhubWidget(Seqhub* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Seqhub.svg")));

    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));



    Contributions* contributions = createWidget<Contributions>(mm2px(Vec(10, 30)));
    contributions->module = module;
    contributions->setSize(mm2px(Vec(183, 53)));
    addChild(contributions);

    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(12, 91)), module, Seqhub::CLOCK_LIGHT));

    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(23, 91)), module, Seqhub::RUN_PARAM, Seqhub::RUN_LIGHT));
    addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(34, 91)), module, Seqhub::RESET_PARAM, Seqhub::RESET_LIGHT));
    addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(45, 91)), module, Seqhub::RANDOM_PARAM, Seqhub::RANDOM_LIGHT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(56, 91)), module, Seqhub::START_DATE_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(67, 91)), module, Seqhub::LENGTH_PARAM));
    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(78, 91)), module, Seqhub::PING_PONG_PARAM, Seqhub::PING_PONG_LIGHT));
    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(89, 91)), module, Seqhub::WORKING_WEEKENDS_PARAM, Seqhub::WORKING_WEEKENDS_LIGHT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(100, 91)), module, Seqhub::RISE_SPEED_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(111, 91)), module, Seqhub::FALL_SPEED_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(122, 91)), module, Seqhub::FILTER_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(133, 91)), module, Seqhub::SCALE_PARAM));

    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(12, 113)), module, Seqhub::CLOCK_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(23, 113)), module, Seqhub::RUN_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(34, 113)), module, Seqhub::RESET_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(45, 113)), module, Seqhub::RANDOM_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(56, 113)), module, Seqhub::START_DATE_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(67, 113)), module, Seqhub::LENGTH_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(78, 113)), module, Seqhub::PING_PONG_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(89, 113)), module, Seqhub::WORKING_WEEKENDS_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(100, 113)), module, Seqhub::RISE_SPEED_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(111, 113)), module, Seqhub::FALL_SPEED_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(122, 113)), module, Seqhub::FILTER_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(133, 113)), module, Seqhub::SCALE_INPUT));

    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(155, 113)), module, Seqhub::CV_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(166, 113)), module, Seqhub::GATE_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(177, 113)), module, Seqhub::TRIGGER_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(188, 113)), module, Seqhub::END_OF_SEQUENCE_OUTPUT));
  }

  void appendContextMenu(ui::Menu* menu) override {
    ModuleWidget::appendContextMenu(menu);

    Seqhub* m = getModule<Seqhub>();
    if (!m) return;

    menu->addChild(new ui::MenuSeparator());
    menu->addChild(createMenuItem("Integrations...", "", [=]() {
      IntegrationsModal* modal = new IntegrationsModal([m](const std::vector<float>& contributions, const std::string& date) {
        m->contributionsPerDay = contributions;
        m->startDate = date;
      });
      APP->scene->addChild(modal);
    }));
  }

};

Model* modelSeqhub = createModel<Seqhub, SeqhubWidget>("Seqhub");
