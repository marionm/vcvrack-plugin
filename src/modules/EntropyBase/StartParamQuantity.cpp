#include "StartParamQuantity.hpp"

float StartParamQuantity::getDisplayValue() {
  return index;
}

void StartParamQuantity::setDisplayValueString(std::string string) {
  try {
    int length = std::stoi(string);
    float value = (float)length / totalLength;
    if (value < 0.f) {
      value = 0.f;
    } else if (value > 1.f) {
      value = 1.f;
    }

    setValue(value);
  } catch (...) {
  }
}
