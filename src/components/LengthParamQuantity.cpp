#include "LengthParamQuantity.hpp"

std::string LengthParamQuantity::getString() {
  if (length >= 0) {
    return getLabel() + ": " + std::to_string(length);
  } else {
    return getLabel() + ": " + std::to_string(-length) + " (reversed)";
  }
}

std::string LengthParamQuantity::getDisplayValueString() {
  return std::to_string(length);
}

void LengthParamQuantity::setDisplayValueString(std::string string) {
  try {
    int length = std::stoi(string);
    float value = (float)length / totalLength;
    if (value < -1.f) {
      value = -1.f;
    } else if (value > 1.f) {
      value = 1.f;
    }

    setValue(value);
  } catch (...) {
  }
}
