#ifndef LIGHT
#define LIGHT

#include "color.h"
#include "point.h"
#include "vector.h"

struct AmbientLight {
  Color color;
};

struct FarLight {
  Color color;
  Vector direction;
};

struct PointLight {
  Color color;
  Point position;
};

#endif  // LIGHT
