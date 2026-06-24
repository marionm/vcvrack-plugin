#include "Grid.hpp"

using namespace rack;

// TODO: Render tooltips by calculating index from mouse position?
void Grid::draw(const DrawArgs &args) {
  for (int i = 0; i < length; i++) {
    int x = 1 + (i % rowLength) * (mm + 1);
    int y = 1 + (i / rowLength) * (mm + 1);
    float value = module ? module->values[i] : defaultDistribution(defaultRng);

    nvgBeginPath(args.vg);
    nvgRoundedRect(args.vg, mm2px(x), mm2px(y), mm2px(mm), mm2px(mm), 3.f);

    // TODO: Play around with colors
    // NVGcolor color = nvgRGBA(86, 211, 100, value * 255.f);
    NVGcolor color = nvgRGBA(46, 160, 67, value * 255.f);
    nvgFillColor(args.vg, color);
    nvgFill(args.vg);

    if (module ? module->isInRange(i) : true) {
      nvgStrokeWidth(args.vg, mm2px(0.35));
      nvgStrokeColor(args.vg, nvgRGBA(255, 255, 255, 60));
      nvgStroke(args.vg);
    }

    if (i == (module ? module->index : 0)) {
      nvgStrokeWidth(args.vg, mm2px(0.5));
      nvgStrokeColor(args.vg, nvgRGB(255, 255, 255));
      nvgStroke(args.vg);
    }
  }
}
