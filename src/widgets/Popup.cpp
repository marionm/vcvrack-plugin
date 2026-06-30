#include "Popup.hpp"

using namespace rack;

Popup::Popup(Widget* widget) : ui::MenuOverlay() {
  center = true;

  content = widget;
  addChild(widget);

  APP->scene->addChild(this);
}

Popup::Popup(Widget* widget, const math::Vec& pos) : Popup(widget) {
  center = false;
  content->box.pos = pos;
}

void Popup::step() {
  if (center) {
    content->box.pos = box.size.minus(content->box.size).div(2.f);
  }

  ui::MenuOverlay::step();
}

void Popup::close(Widget* widget) {
  Popup* popup = widget->getAncestorOfType<Popup>();
  if (popup) {
    popup->requestDelete();
  }
}
