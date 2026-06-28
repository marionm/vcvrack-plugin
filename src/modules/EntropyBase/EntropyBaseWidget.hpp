#include "EntropyBase.hpp"

#include <rack.hpp>

#include <string>

struct EntropyBaseWidget : rack::app::ModuleWidget {
  EntropyBaseWidget(EntropyBase* module, std::string svgPath);

  void appendContextMenu(rack::ui::Menu* menu) override;
};
