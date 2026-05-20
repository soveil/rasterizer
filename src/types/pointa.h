#ifndef POINTA
#define POINTA

#include "color.h"
#include "point.h"
#include "pointh.h"
#include "vector.h"

class PointA {
 public:
  PointH pointh;
  Color color;
  Vector normal_vector;
  float k;
  float s;
  float t;
  Point world_point;

  PointA() : PointA(0, 0, 0) {}

  PointA(const float point[3]) : PointA(point[0], point[1], point[2]) {}

  PointA(const float point[3], const Color color) : PointA(point[0], point[1], point[2], color) {}

  PointA(const float point[3], const Color color, const Vector normal_vector) : PointA(point[0], point[1], point[2], color, normal_vector) {}

  PointA(const float x, const float y, const float z) : PointA(x, y, z, 1) {}

  PointA(const float x, const float y, const float z, const Color color) : PointA(x, y, z, 1) { this->color = color; }

  PointA(const float x, const float y, const float z, const Color color, const Vector normal_vector) : PointA(x, y, z, 1) {
    this->color = color;
    this->normal_vector = normal_vector;
  }

  PointA(const float x, const float y, const float z, const float w);

  PointA(const PointH pointh, const Color color, const Vector normal_vector, const float k, const float s, const float t, const Point world_point);

  PointA& operator*=(const float other);
  PointA& operator/=(const float other);

  PointA& operator+=(const PointA& other);
  PointA& operator-=(const PointA& other);

  friend PointA operator+(PointA lhs, const PointA& rhs) { return lhs += rhs; }

  friend PointA operator-(PointA lhs, const PointA& rhs) { return lhs -= rhs; }

  void normalize();

  friend PointA operator*(const float lhs, PointA rhs) { return rhs *= lhs; }

  friend PointA operator/(PointA lhs, const float rhs) { return lhs /= rhs; }

  friend PointA operator*(PointA lhs, const float rhs) { return lhs *= rhs; }
};

#endif  // !POINTA
