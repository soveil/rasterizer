#include "point.h"

#include "vector.h"

Point operator*(Point lhs, const float rhs) {
  return lhs *= rhs;
}

Point operator*(float lhs, const Point& rhs) {
  return rhs * lhs;
}

Vector operator-(Point lhs, const Point& rhs) {
  return Vector(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

Vector Point::vector_to(const Point& to) {
  return (to - *this);
}
