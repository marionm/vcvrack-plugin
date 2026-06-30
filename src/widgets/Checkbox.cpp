#include "Checkbox.hpp"

using namespace rack;

// Trying to use VCVRack widgets feels even worse than just drawing a checkbox manually
void Checkbox::draw(const DrawArgs& args) {
  nvgBeginPath(args.vg);
  nvgRoundedRect(args.vg, 0, 0, 15, 15, 3.f);
  nvgStrokeWidth(args.vg, 1.f);
  nvgStrokeColor(args.vg, nvgRGB(100, 100, 100));
  nvgStroke(args.vg);

  if (value) {
    nvgBeginPath(args.vg);
    nvgRoundedRect(args.vg, 3, 3, 9, 9, 1.5f);
    nvgFillColor(args.vg, nvgRGB(255, 255, 255));
    nvgFill(args.vg);
  }

  nvgFillColor(args.vg, nvgRGB(255, 255, 255));
  nvgFontSize(args.vg, 13.f);
  nvgFontFaceId(args.vg, APP->window->uiFont->handle);
  nvgTextAlign(args.vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
  nvgText(args.vg, 22, 7.5, label.c_str(), nullptr);
}

void Checkbox::onButton(const event::Button& e) {
  if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS) {
    value = !value;
    e.consume(this);
  }
}
