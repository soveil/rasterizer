#include "math.h"

#include <cmath>

bool Math::equal_colors(const Color& a, const Color& b) {
  const double margin = 0.01;
  if (std::abs(a.r - b.r) > margin) {
    return false;
  }
  if (std::abs(a.g - b.g) > margin) {
    return false;
  }
  if (std::abs(a.b - b.b) > margin) {
    return false;
  }
  return true;
}

void Math::round_to_int_array(const float* float_arr, int* round_arr, int size) {
  for (int i = 0; i < size; ++i) {
    round_arr[i] = std::round(float_arr[i]);
  }
}

void Math::round_to_float_array(const float* float_arr, float* round_arr, int size, int place) {
  float modifier = std::pow(10, place);
  for (int i = 0; i < size; ++i) {
    round_arr[i] = std::round(float_arr[i] * modifier) / modifier;
  }
}

float Math::deg_to_rad(const float degree) {
  return degree * M_PI / 180.0;
}
