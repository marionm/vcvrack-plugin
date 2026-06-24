#pragma once

#include "../modules/EntropyBase.hpp"

#include <rack.hpp>

#include <random>

struct Grid : rack::Widget {
  int length, rowLength, mm;

  // Will be null in previews - use default rng to generate a preview grid
  EntropyBase* module = nullptr;
  std::mt19937 defaultRng{42u};
  std::uniform_real_distribution<float> defaultDistribution{0.f, 1.f};

  void draw(const DrawArgs &args) override;
};

