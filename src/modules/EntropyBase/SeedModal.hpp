#pragma once

#include "../../widgets/Modal.hpp"

#include <functional>

#include <rack.hpp>

struct SeedModal : Modal {
  SeedModal(uint32_t seed, std::function<void(uint32_t)> onSave);

private:
  std::function<void(uint32_t)> onSave;

  rack::ui::TextField* seedField = nullptr;
  rack::ui::Label* statusLabel = nullptr;
};
