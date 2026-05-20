#ifndef POINTH
#define POINTH

#include <string>

#include "vector.h"
#include "xform.h"

class PointH {
 public:
  union {
    struct {
      float x;
      float y;
      float z;
      float w;
    };

    struct {
      float array[4];
    };
  };

  PointH() {
    x = 0;
    y = 0;
    z = 0;
    w = 1;
  }

  void normalize() {
    x = get_x();
    y = get_y();
    z = get_z();
    w = 1;
  }

  PointH normalized() const {
    PointH p = *this;
    p.normalize();
    return p;
  }

  PointH(const float x, const float y, const float z) : PointH(x, y, z, 1) {}

  PointH(const float point[3]) : PointH(point[0], point[1], point[2]) {}

  PointH(const float x, const float y, const float z, const float w) {
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
  }

  bool operator<(const PointH& other) const;

  float get_x() const;

  float get_y() const;

  float get_z() const;

  std::string to_string() const;
  Vector to_vector() const;

  PointH& operator+=(const PointH& other);
  PointH& operator-=(const PointH& other);
  PointH& operator+=(const Vector& other);

  PointH& operator+=(const float other);
  PointH& operator*=(const float other);
  PointH& operator/=(const float other);

  // returns the vector that goes from this point to the to point
  Vector vector_to(const PointH& to) const;

  /*
   * Matrix multiplication with a heterogeneous point
   * Assumes matrix has 4 rows and columns
   */
  friend PointH operator*(const Xform& lhs, const PointH& rhs) {
    PointH p;

    for (int r = 0; r < XformRows; ++r) {
      p.array[r] = (lhs.get(r, 0) * rhs.x) + (lhs.get(r, 1) * rhs.y) + (lhs.get(r, 2) * rhs.z) + (lhs.get(r, 3) * rhs.w);
    }
    return p;
  }

  friend PointH operator*(float lhs, PointH rhs) { return rhs *= lhs; }

  friend PointH operator*(PointH lhs, float rhs) { return lhs *= rhs; }

  friend PointH operator/(PointH lhs, const float rhs) { return lhs /= rhs; }

  friend PointH operator+(PointH lhs, const PointH& rhs) { return lhs += rhs; }

  friend PointH operator-(PointH lhs, const PointH& rhs) { return lhs -= rhs; }
};

#endif  // !POINTH
