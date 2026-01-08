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
    ENUMS(REFRESH_LIGHT, 2),
    NUM_LIGHTS
  };

  enum class RefreshStatus {
    Idle,
    InProgress,
    Error
  };

  static const auto REFRESH_LIGHT_G = REFRESH_LIGHT;
  static const auto REFRESH_LIGHT_R = REFRESH_LIGHT + 1;

  std::string auth;

  std::vector<u_int> contributionsPerDay;
  std::string startDate;

  std::thread worker;
  std::atomic<bool> stopWorker{false};
  std::atomic<bool> shouldFetch{false};
  std::atomic<RefreshStatus> refreshStatus{RefreshStatus::Idle};

  Seqhub() {
    config(0, 0, 0, NUM_LIGHTS);

    worker = std::thread([this] { workerLoop(); });
  }

  ~Seqhub() override {
    stopWorker.store(true);
    if (worker.joinable()) {
      worker.join();
    }
  }

  void process(const ProcessArgs&) override {
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

    refreshStatus.store(RefreshStatus::InProgress);

    try {
      size_t atPos = auth.find('@');
      size_t splitIndex = (atPos == std::string::npos) ? 0 : atPos + 1;
      std::string username = (atPos == std::string::npos) ? "" : auth.substr(0, atPos);
      std::string token = auth.substr(splitIndex);

      if (token.empty()) {
        refreshStatus.store(RefreshStatus::Error);
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
      if (module->refreshStatus.load() != Seqhub::RefreshStatus::InProgress) {
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

struct RefreshButton : TGreenRedLight<GrayModuleLightWidget> {
  Seqhub* module = nullptr;
  AuthField* authField = nullptr;

  void onButton(const rack::event::Button& e) override {
    if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS) {
      if (module && module->refreshStatus.load() != Seqhub::RefreshStatus::InProgress) {
        module->auth = authField->text;
        module->shouldFetch.store(true);
      }

      e.consume(this);
    }

    GrayModuleLightWidget::onButton(e);
  }
};

struct SeqhubWidget : app::ModuleWidget {
  SeqhubWidget(Seqhub* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Seqhub.svg")));

    auto* authField = new AuthField();
    authField->module = module;
    authField->box.pos = mm2px(Vec(5, 20));
    authField->box.size = mm2px(Vec(50, 8));
    addChild(authField);

    RefreshButton* refreshButton = createLight<RefreshButton>(mm2px(Vec(5, 32)), module, Seqhub::REFRESH_LIGHT);
    refreshButton->module = module;
    refreshButton->authField = authField;
    refreshButton->box.size = mm2px(Vec(5.3, 5.3)); 

    addChild(refreshButton);
  }

  void step() override {
    ModuleWidget::step();

    Seqhub* m = getModule<Seqhub>();
    if (!m) {
      return;
    }

    switch (m->refreshStatus.load()) {
      case Seqhub::RefreshStatus::Idle:
        m->lights[Seqhub::REFRESH_LIGHT_G].setBrightness(0.f);
        m->lights[Seqhub::REFRESH_LIGHT_R].setBrightness(0.f);
        break;
      case Seqhub::RefreshStatus::InProgress:
        m->lights[Seqhub::REFRESH_LIGHT_G].setBrightness(0.5f);
        m->lights[Seqhub::REFRESH_LIGHT_R].setBrightness(1.f);
        break;
      case Seqhub::RefreshStatus::Error:
        m->lights[Seqhub::REFRESH_LIGHT_G].setBrightness(0.f);
        m->lights[Seqhub::REFRESH_LIGHT_R].setBrightness(1.f);
        break;
    }
  }
};

Model* modelSeqhub = createModel<Seqhub, SeqhubWidget>("Seqhub");
