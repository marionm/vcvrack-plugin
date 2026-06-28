#include "ScaleParamQuantity.hpp"

using namespace rack;

std::string ScaleParamQuantity::getString() {
  float value = getValue();

  if (value == 0) {
    return getLabel() + ": 0";
  } else if (value > 0) {
    return string::f("%s: 0 to %.2f", getLabel(), value * 10.f);
  } else {
    return string::f("%s: %.2f to %.2f", getLabel(), value * 5.f, value * -5.f);
  }
}
