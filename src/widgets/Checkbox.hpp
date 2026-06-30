#pragma once

#include <rack.hpp>

#include <string>

// TODO: Autosize based on label
struct Checkbox : rack::widget::OpaqueWidget {
  std::string label;
  bool value = false;

private:
  void draw(const DrawArgs& args) override;
  void onButton(const rack::event::Button& e) override;
};
