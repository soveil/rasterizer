#include "xform.h"

#include <cmath>
#include <string>
#include "../math.h"

const int XFORM_SIDE = 4;

Xform::Xform() {
  array = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
}

Xform::Xform(Xform& other) {
  array = other.array;
}

Xform& Xform::operator=(Xform other) {
  array = other.array;
  return *this;
}

Xform::Xform(XformArray array) {
  this->array = array;
}

Xform Xform::operator*=(XformArray rhs) {
  XformArray res = {};

  // Iterate through result matrix
  for (int r = 0; r < XFORM_SIDE; ++r) {
    for (int c = 0; c < XFORM_SIDE; ++c) {

      // Sum matrix cells together into result matrix
      for (int mut_cell = 0; mut_cell < XFORM_SIDE; ++mut_cell) {
        res[r * XFORM_SIDE + c] += get(r, mut_cell) * rhs[mut_cell * XFORM_SIDE + c];
      }
    }
  }

  array = res;
  return *this;
}

float& Xform::operator()(int row, int col) {
  return array[row * XformCols + col];
}

float Xform::get(int row, int col) const {
  return array[row * XformCols + col];
}

Xform Xform::identity() {
  Xform x;
  x.array[0] = 1;
  x.array[5] = 1;
  x.array[10] = 1;
  x.array[15] = 1;
  return x;
}

void Xform::translate(float tx, float ty, float tz) {
  // clang-format off
  *this *= XformArray{
      1, 0, 0, tx,
      0, 1, 0, ty,
      0, 0, 1, tz,
      0, 0, 0, 1
  };
  // clang-format on
}

void Xform::scale(float sx, float sy, float sz) {
  *this *= XformArray{ sx, 0, 0, 0, 0, sy, 0, 0, 0, 0, sz, 0, 0, 0, 0, 1 };
}

// TODO: cache angles?
void Xform::rotate_xy(float angle) {
  float rad_angle = Math::deg_to_rad(angle);
  float cos_angle = std::cos(rad_angle);
  float sin_angle = std::sin(rad_angle);
  *this *= XformArray{ cos_angle, -sin_angle, 0, 0, sin_angle, cos_angle, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
}

void Xform::rotate_yz(float angle) {
  float rad_angle = Math::deg_to_rad(angle);
  float cos_angle = std::cos(rad_angle);
  float sin_angle = std::sin(rad_angle);
  *this *= XformArray{ 1, 0, 0, 0, 0, cos_angle, -sin_angle, 0, 0, sin_angle, cos_angle, 0, 0, 0, 0, 1 };
}

void Xform::rotate_zx(float angle) {
  float rad_angle = Math::deg_to_rad(angle);
  float cos_angle = std::cos(rad_angle);
  float sin_angle = std::sin(rad_angle);
  *this *= XformArray{ cos_angle, 0, sin_angle, 0, 0, 1, 0, 0, -sin_angle, 0, cos_angle, 0, 0, 0, 0, 1 };
}

std::string Xform::to_string() const {
  // clang-format off
  return "Xform:\n |" +
    std::to_string(array[0]) + ", " + std::to_string(get(0, 1)) + ", " + std::to_string(get(0, 2)) + ", " + std::to_string(get(0, 3)) + "|\n |" +
    std::to_string(get(1, 0)) + ", " + std::to_string(get(1, 1)) + ", " + std::to_string(get(1, 2)) + ", " + std::to_string(get(1, 3)) + "|\n |" +
    std::to_string(get(2, 0)) + ", " + std::to_string(get(2, 1)) + ", " + std::to_string(get(2, 2)) + ", " + std::to_string(get(2, 3)) + "|\n |" +
    std::to_string(get(3, 0)) + ", " + std::to_string(get(3, 1)) + ", " + std::to_string(get(3, 2)) + ", " + std::to_string(get(3, 3)) + "|";
  // clang-format on
}
