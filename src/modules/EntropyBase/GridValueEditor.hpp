#pragma once

#include <rack.hpp>

struct GridValueEditor : rack::ui::Menu {
  GridValueEditor(int index, float* pValue);

private:
  rack::ui::TextField* input;
};

