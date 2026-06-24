#include "EntropyBase.hpp"
#include "../plugin.hpp"

#include <string>

#include <rack.hpp>

struct EntropyBaseWidget : rack::app::ModuleWidget {
  EntropyBaseWidget(EntropyBase* module, std::string svgPath);
  void appendContextMenu(rack::ui::Menu* menu) override;
};
