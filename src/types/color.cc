#include "color.h"

Color::Color(float r, float g, float b) {
  this->r = r;
  this->g = g;
  this->b = b;
}

Color Color::operator=(const float color[3]) {
  r = color[0];
  g = color[1];
  b = color[2];
  return *this;
}

Color Color::operator*=(const Color& rhs) {
  r *= rhs.r;
  g *= rhs.g;
  b *= rhs.b;
  return *this;
}

Color Color::operator+=(const Color& rhs) {
  r += rhs.r;
  g += rhs.g;
  b += rhs.b;
  return *this;
}

Color Color::operator-=(const Color& rhs) {
  r -= rhs.r;
  g -= rhs.g;
  b -= rhs.b;
  return *this;
}

Color Color::operator*=(const float rhs) {
  r *= rhs;
  g *= rhs;
  b *= rhs;
  return *this;
}

Color Color::operator/=(const float rhs) {
  r /= rhs;
  g /= rhs;
  b /= rhs;
  return *this;
}

std::string Color::to_string() const {
  return "Color: ( " + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + " )";
}
