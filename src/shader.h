#ifndef SHADER
#define SHADER

#include "types/color.h"
#include "types/vector.h"

class Shader {
 public:
  static Color matte(const Color& c);
  static Color metal(const Color& c);
  static Color plastic(const Color& c);

 private:
  // use vertex normal if use_interpolated and vertices have normals, otherwise use polygon normals
  static Vector get_normal();

  static Color ambient(const Color& c);
  static Color diffuse(const Color& c);
  static Color specular(const Color& c, const Color& Lc, const Vector& L, const Vector& N);
};

#endif  // SHADER
