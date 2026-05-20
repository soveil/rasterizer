#ifndef POINT
#define POINT

#include <string>

#include "pointh.h"

class Point {
 public:
  union {
    struct {
      float x;
      float y;
      float z;
    };

    struct {
      float array[3];
    };
  };

  Point() : Point(0, 0, 0) {}

  Point(const PointH& pointh) {
    x = pointh.get_x();
    y = pointh.get_y();
    z = pointh.get_z();
  }

  Point(const float x, const float y, const float z) {
    this->x = x;
    this->y = y;
    this->z = z;
  }

  Point& operator+=(const float rhs) {
    x += rhs;
    y += rhs;
    z += rhs;
    return *this;
  }

  Point& operator+=(const Point& rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
  }

  Point& operator-=(const Point& rhs) {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
  }

  Point& operator*=(const float rhs) {
    x *= rhs;
    y *= rhs;
    z *= rhs;
    return *this;
  }

  Point& operator/=(const float rhs) {
    x /= rhs;
    y /= rhs;
    z /= rhs;
    return *this;
  }

  Vector vector_to(const Point& to);

  std::string to_string() const { return "Point: (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")"; }

  Vector to_vector() const { return Vector(x, y, z); }

  friend Point operator+(Point lhs, const float rhs) { return lhs += rhs; }
};

#endif  // !POINT
