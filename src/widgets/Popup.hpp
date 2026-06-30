#pragma once

#include <rack.hpp>

// Wraps a single widget and positions it on screen, closing it when anything else is clicked
// Just instantiate to spawn, and call Popup::close(yourWidget) to manually close
struct Popup : rack::ui::MenuOverlay {
  // Centers the widget on screen
  Popup(Widget* widget);

  // Uses the given position instead of centering
  Popup(Widget* widget, const rack::math::Vec& position);

  static void close(Widget* widget);

private:
  rack::widget::Widget* content = nullptr;
  bool center = false;

  void step() override;
};
