#include "vector.h"

#include <cmath>
#include <string>

Vector::Vector(float i, float j, float k) {
  this->i = i;
  this->j = j;
  this->k = k;
}

Vector& Vector::operator*=(const float rhs) {
  i *= rhs;
  j *= rhs;
  k *= rhs;
  return *this;
}

Vector& Vector::operator+=(const float rhs) {
  i += rhs;
  j += rhs;
  k += rhs;
  return *this;
}

Vector& Vector::operator/=(const float rhs) {
  i /= rhs;
  j /= rhs;
  k /= rhs;
  return *this;
}

Vector& Vector::operator+=(const Vector& rhs) {
  i += rhs.i;
  j += rhs.j;
  k += rhs.k;
  return *this;
}

Vector& Vector::operator-=(const Vector& rhs) {
  i -= rhs.i;
  j -= rhs.j;
  k -= rhs.k;
  return *this;
}

Vector& Vector::operator-() {
  i = -i;
  j = -j;
  k = -k;
  return *this;
}

Vector Vector::operator-() const {
  Vector v = *this;
  return -v;
}

float Vector::dot(const Vector& rhs) const {
  return i * rhs.i + j * rhs.j + k * rhs.k;
}

Vector Vector::cross(const Vector& rhs) const {
  Vector v;
  v.i = j * rhs.k - k * rhs.j;
  v.j = k * rhs.i - i * rhs.k;
  v.k = i * rhs.j - j * rhs.i;
  return v;
}

float Vector::mag2() const {
  return i * i + j * j + k * k;
}

float Vector::mag() const {
  return std::sqrt(mag2());
}

void Vector::normalize() {
  float m = mag();
  if (m != 0) {
    i /= m;
    j /= m;
    k /= m;
  }
}

Vector Vector::normalized() const {
  Vector v = *this;
  v.normalize();
  return v;
}

Vector Vector::reflected(const Vector& normal) const {
  return (2 * normal * this->dot(normal)) - *this;
}

std::string Vector::to_string() const {
  return "Vector: (" + std::to_string(i) + ", " + std::to_string(j) + ", " + std::to_string(k) + ")";
}
