#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "plugin.hpp"

#include <atomic>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace rack;

// Sequencer driven by Github contribution activity
struct Seqhub : Module {
  enum LightIds {
    FETCH_LIGHT,
    NUM_LIGHTS
  };

  enum class FetchState {
    Idle,
    InProgress,
    Success,
    Error
  };

  std::string auth;

  std::vector<u_int> contributionsPerDay;
  std::string startDate;

  std::thread worker;
  std::atomic<bool> stopWorker{false};
  std::atomic<bool> shouldFetch{false};
  std::atomic<FetchState> fetchState{FetchState::Idle};

  Seqhub() {
    config(0, 0, 0, NUM_LIGHTS);

    configLight(FETCH_LIGHT);

    worker = std::thread([this] { workerLoop(); });
  }

  ~Seqhub() override {
    stopWorker.store(true);
    if (worker.joinable()) {
      worker.join();
    }
  }

  void process(const ProcessArgs&) override {
    switch (fetchState.load()) {
      case FetchState::Idle:
        lights[FETCH_LIGHT].setBrightness(0.f);
        break;
      case FetchState::InProgress:
        lights[FETCH_LIGHT].setBrightness(0.5f);
        break;
      case FetchState::Success:
        lights[FETCH_LIGHT].setBrightness(1.f);
        break;
      case FetchState::Error:
        lights[FETCH_LIGHT].setBrightness(-1.f);
        break;
    }
  }

  void workerLoop() {
    while (!stopWorker.load()) {
      if (shouldFetch.exchange(false)) {
        fetchContributions();
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  }

  void fetchContributions() {
    if (auth.empty()) {
      refreshStatus.store(RefreshStatus::Idle);
      return;
    }

    fetchState.store(FetchState::InProgress);

    try {
      size_t atPos = auth.find('@');
      size_t splitIndex = (atPos == std::string::npos) ? 0 : atPos + 1;
      std::string username = (atPos == std::string::npos) ? "" : auth.substr(0, atPos);
      std::string token = auth.substr(splitIndex);

      if (token.empty()) {
        fetchState.store(FetchState::Error);
        return;
      }

      httplib::SSLClient cli("api.github.com");

      httplib::Headers headers = {
        {"Authorization", "Bearer " + token},
        {"Content-Type", "application/json"},
        {"User-Agent", "cpp-httplib-plugin"}
      };

      std::string header, requestScope, responseScope;
      if (username.empty()) {
        header = "query";
        requestScope = "viewer";
        responseScope = "viewer";
      } else {
        header = "query($username: String!)";
        requestScope = "user(login: $username)";
        responseScope = "user";
      }

      std::string query = R"(
        contributionsCollection {
          startedAt
          contributionCalendar {
            weeks {
              contributionDays {
                contributionCount
              }
            }
          }
        }
      )";

      std::string body = header + " { " + requestScope + " { " + query + " } }";

      nlohmann::json jsonBody;
      jsonBody["query"] = body;
      if (!username.empty()) {
        jsonBody["variables"] = {{"username", username}};
      }

      DEBUG("request:\n%s", body.c_str());

      if (auto res = cli.Post("/graphql", headers, jsonBody.dump(), "application/json")) {
        if (res->status == 200) {
          auto json = nlohmann::json::parse(res->body);
          auto& contributions = json["data"][responseScope]["contributionsCollection"];

          startDate = contributions["startedAt"].get<std::string>();

          contributionsPerDay.clear();
          for (auto& week : contributions["contributionCalendar"]["weeks"]) {
            for (auto& day : week["contributionDays"]) {
              contributionsPerDay.push_back(day["contributionCount"].get<int>());
            }
          }

          logState("refreshContributions");
          refreshStatus.store(RefreshStatus::Idle);
          return;
        }
      }
    } catch (...) {
    }

    refreshStatus.store(RefreshStatus::Error);
  }

  // Token is potentially sensitive - don't hang onto it
  json_t* dataToJson() override {
    json_t* root = json_object();

    json_object_set_new(root, "startDate", json_string(startDate.c_str()));

    // TODO: Use patch storage instead of JSON?
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

    if (!contributionsPerDay.empty()) {
      fetchState.store(FetchState::Success);
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

struct AuthField : ui::TextField {
  Seqhub* module = nullptr;

  void onSelectKey(const rack::event::SelectKey& e) override {
    TextField::onSelectKey(e);

    if (e.action == GLFW_PRESS && e.key == GLFW_KEY_ENTER) {
      if (module->fetchState.load() != Seqhub::FetchState::InProgress) {
        module->auth = text;
        module->shouldFetch.store(true);
      }
    }
  }

  void draw(const DrawArgs& args) override {
    size_t atPos = text.find('@');
    size_t splitIndex = (atPos == std::string::npos) ? 0 : atPos + 1;
    std::string usernameWithAt = text.substr(0, splitIndex);
    std::string token = text.substr(atPos + 1);

    std::string originalText = text;
    text = usernameWithAt + std::string(token.size(), '*');
    TextField::draw(args);
    text = originalText;
  }
};

struct PassFailLight : MediumLight<GreenRedLight> {
  void draw(const DrawArgs &args) override {
    if (module) {
      float brightness = module->lights[Seqhub::FETCH_LIGHT].getBrightness();

      if (brightness < 0.f) {
        setBrightnesses({0.f, 1.f});
      } else {
        setBrightnesses({brightness, 0.f});
      }
    }

    MediumLight<GreenRedLight>::draw(args);
  }
};

struct SeqhubWidget : app::ModuleWidget {
  ui::Label* startDateLabel;

  SeqhubWidget(Seqhub* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Seqhub.svg")));

    auto* authField = new AuthField();
    authField->module = module;
    authField->box.pos = mm2px(Vec(5, 20));
    authField->box.size = mm2px(Vec(50, 8));
    addChild(authField);

    addChild(createLightCentered<PassFailLight>(mm2px(Vec(30, 58)), module, Seqhub::FETCH_LIGHT));

    startDateLabel = new ui::Label();
    startDateLabel->box.pos = mm2px(Vec(5, 44)); // Position it below the token
    startDateLabel->text = "Start date: " + (module ? module->startDate : "");
    addChild(startDateLabel);
  }

  void step() override {
    ModuleWidget::step();

    if (module) {
      Seqhub* m = dynamic_cast<Seqhub*>(module);
      if (m && startDateLabel) {
        startDateLabel->text = "Start date: " + m->startDate;
      }
    }
  }
};

Model* modelSeqhub = createModel<Seqhub, SeqhubWidget>("Seqhub");
