#include "pointh.h"

#include <string>

PointH& PointH::operator+=(const float other) {
  x += other;
  y += other;
  z += other;
  w += other;
  return *this;
}

PointH& PointH::operator*=(const float other) {
  x *= other;
  y *= other;
  z *= other;
  w *= other;
  return *this;
}

PointH& PointH::operator/=(const float other) {
  x /= other;
  y /= other;
  z /= other;
  w /= other;
  return *this;
}

PointH& PointH::operator+=(const PointH& other) {
  x += other.x;
  y += other.y;
  z += other.z;
  w += other.w;
  return *this;
}

PointH& PointH::operator-=(const PointH& other) {
  x -= other.x;
  y -= other.y;
  z -= other.z;
  w -= other.w;
  return *this;
}

bool PointH::operator<(const PointH& other) const {
  if (get_x() < other.get_x()) {
    return true;
  }
  if (get_y() < other.get_y()) {
    return true;
  }
  return get_z() < other.get_z();
}

float PointH::get_x() const {
  return x / w;
}

float PointH::get_y() const {
  return y / w;
}

float PointH::get_z() const {
  return z / w;
}

Vector PointH::vector_to(const PointH& to) const {
  return (to - *this).to_vector();
}

std::string PointH::to_string() const {
  return "PointH: (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ", " + std::to_string(w) + ") : (" + std::to_string(get_x()) + ", " + std::to_string(get_y()) + ", " +
         std::to_string(get_z()) + ")";
}

Vector PointH::to_vector() const {
  return Vector(x, y, z);
}
