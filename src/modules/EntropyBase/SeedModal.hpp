#pragma once

#include "EntropyBase.hpp"
#include "../../widgets/Modal.hpp"

#include <rack.hpp>

struct SeedModal : Modal {
  SeedModal(EntropyBase* module);

  void onOpen() override;
  bool onSave() override;

private:
  EntropyBase* module;
  rack::ui::TextField* seedField;
  rack::ui::Label* statusLabel;
};
