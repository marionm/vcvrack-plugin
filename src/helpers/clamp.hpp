# pragma once

#include <algorithm>

inline float clamp01(float value) {
  return std::max(0.f, std::min(value, 1.f));
}

inline float clamp11(float value) {
  return std::max(-1.f, std::min(value, 1.f));
}
