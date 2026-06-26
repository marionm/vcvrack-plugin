#include "EntropyBase.hpp"
#include "../plugin.hpp"

#include <string>

struct EntropyBaseWidget : rack::app::ModuleWidget {
  EntropyBaseWidget(EntropyBase* module, std::string svgPath);

  void appendContextMenu(rack::ui::Menu* menu) override;
};
