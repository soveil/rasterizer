#ifndef VEC
#define VEC

#include <string>

#include "xform.h"

constexpr int VECTOR_SIZE = 3;

class Vector {
 public:
  union {
    struct {
      float i;
      float j;
      float k;
    };

    struct {
      float x;
      float y;
      float z;
    };

    struct {
      float array[VECTOR_SIZE];
    };
  };

  Vector() : Vector(0.) {}

  Vector(float i) : Vector(i, i, i) {}

  Vector(const float array[3]) : Vector(array[0], array[1], array[2]) {}

  Vector(float i, float j, float k);

  Vector& operator*=(const float rhs);

  Vector& operator+=(const float rhs);

  Vector& operator/=(const float rhs);

  Vector& operator+=(const Vector& rhs);

  Vector& operator-=(const Vector& rhs);

  Vector& operator-();
  Vector operator-() const;

  float dot(const Vector& rhs) const;

  Vector cross(const Vector& rhs) const;

  float mag2() const;

  float mag() const;

  void normalize();

  Vector normalized() const;

  Vector reflected(const Vector& normal) const;

  std::string to_string() const;

  friend Vector operator*(Vector lhs, const float rhs) { return lhs *= rhs; }

  friend Vector operator/(Vector lhs, const float rhs) { return lhs /= rhs; }

  friend Vector operator*(const float lhs, Vector rhs) { return rhs *= lhs; }

  friend Vector operator+(Vector lhs, const Vector& rhs) { return lhs += rhs; }

  friend Vector operator-(Vector lhs, const Vector& rhs) { return lhs -= rhs; }

  // assumes w is 0
  friend Vector operator*(const Vector& lhs, const Xform& rhs) {
    Vector v;

    for (int c = 0; c < VECTOR_SIZE; ++c) {
      v.array[c] = (lhs.i * rhs.get(0, c)) + (lhs.j * rhs.get(1, c)) + (lhs.k * rhs.get(2, c));
    }

    return v;
  }

  // assumes w is 0
  friend Vector operator*(const Xform& lhs, const Vector& rhs) {
    Vector v;

    for (int r = 0; r < VECTOR_SIZE; ++r) {
      v.array[r] = (lhs.get(r, 0) * rhs.i) + (lhs.get(r, 1) * rhs.j) + (lhs.get(r, 2) * rhs.k);
    }

    return v;
  }
};

#endif  // !VEC
