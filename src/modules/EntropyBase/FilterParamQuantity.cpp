#include "FilterParamQuantity.hpp"

std::string FilterParamQuantity::getString() {
  float value = getValue();

  if (value == 0.f) {
    return getLabel() + ": None";
  } else if (value > 0.f) {
    return getLabel() + " >= " + std::to_string(value);
  } else {
    return getLabel() + " <= " + std::to_string(value + 1);
  }
}

std::string FilterParamQuantity::getDisplayValueString() {
  float value = getValue();

  if (value >= 0.f) {
    return std::to_string(value);
  } else {
    return "-" + std::to_string(value + 1);
  }
}

void FilterParamQuantity::setDisplayValueString(std::string string) {
  try {
    float value = std::stof(string);

    if (value < 0.f) {
      value = -1.f - value;
    }

    if (value < -1.f) {
      value = -1.f;
    } else if (value > 1.f) {
      value = 1.f;
    }

    setValue(value);
  } catch(...) {
  }
}
