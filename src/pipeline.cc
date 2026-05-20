#include "pipeline.h"

#include <cmath>

#if DEBUG >= 1
#include <iostream>
#endif

#include "globals.h"
#include "math.h"
#include "rd_display.h"
#include "types/vector.h"
#include "types/xform.h"

const float EPSILON = 0.001;

Xform Pipeline::world_to_camera() {
  Vector E = g::camera_eye.to_vector();
  Vector A = g::camera_eye.vector_to(g::camera_at);
  A.normalize();
  Vector U = A.cross(g::camera_up);
  U.normalize();
  Vector V = U.cross(A);

#if DEBUG >= 3
  std::cout << "camera_eye: " << camera_eye.to_string() << std::endl;
  std::cout << "camera_at: " << camera_at.to_string() << std::endl;
  std::cout << "camera_up: " << camera_up.to_string() << std::endl;
  std::cout << "E = " << E.to_string() << std::endl;
  std::cout << "A = " << A.to_string() << std::endl;
  std::cout << "U = " << U.to_string() << std::endl;
  std::cout << "V = " << V.to_string() << std::endl;
#endif

  // clang-format off
  Xform rotation = Xform({
      U.x, U.y, U.z, 0,
      V.x, V.y, V.z, 0,
      A.x, A.y, A.z, 0,
      0,     0,   0, 1
  });

  Xform translation = Xform({
      1, 0, 0, -E.x,
      0, 1, 0, -E.y,
      0, 0, 1, -E.z,
      0, 0, 0,    1
  });
  // clang-format on

  return rotation * translation;
}

Xform Pipeline::camera_to_clip() {
  float a = float(display_xSize) / display_ySize;  // aspect
  float tan_fov_2 = std::tan(Math::deg_to_rad(g::camera_fov) / 2);

#if DEBUG >= 1
  std::cout << "aspect: " << a << std::endl;
  std::cout << "fov: " << camera_fov << std::endl;
  std::cout << "near_clip: " << near_clip << std::endl;
  std::cout << "far_clip: " << far_clip << std::endl;
  std::cout << "tan(fov / 2): " << tan_fov_2 << std::endl;
#endif

  // clang-format off
  return Xform({
      1 / (2 * a * tan_fov_2),                   0,                                          0.5,                                                            0,
                            0, 1 / (2 * tan_fov_2),                                          0.5,                                                            0,
                            0,                   0, (g::far_clip) / (g::far_clip - g::near_clip), -(g::far_clip * g::near_clip) / (g::far_clip - g::near_clip),
                            0,                   0,                                            1,                                                            0
  });
  // clang-format on
}

Xform Pipeline::clip_to_device() {
  // clang-format off
  return Xform({
      static_cast<float>(display_xSize) - EPSILON, 0, 0, -0.5,
      0, -(static_cast<float>(display_ySize) - EPSILON), 0, static_cast<float>(display_ySize) - EPSILON - 0.5f,
      0, 0, 1, 0,
      0, 0, 0, 1
  });
  // clang-format on
}
