#pragma once

#include <rack.hpp>

struct GridValueEditor : rack::ui::TextField {
  static void spawn(int index, float* targetFloat, const rack::math::Vec& position);

  float* pValue;
  bool wasFocused = false;

  GridValueEditor(float* pValue);

  void onAction(const ActionEvent& e) override;
  void draw(const DrawArgs& args) override;
};

