#ifndef GLOBALS
#define GLOBALS

#include <stack>
#include <vector>

#include "shader.h"
#include "types/color.h"
#include "types/edge.h"
#include "types/light.h"
#include "types/point.h"
#include "types/pointh.h"
#include "types/vector.h"
#include "types/xform.h"

namespace g {
constexpr int DISK_SEGMENTS = 20;
constexpr int MAX_POLYGON_VERTICES = 50;
constexpr int MAX_POLYGON_EDGES = MAX_POLYGON_VERTICES;

inline Color fg_color = Color(1);
inline Color bg_color = Color();

inline int frame_number = 0;

inline float camera_fov = 90;
inline float near_clip = 1.0;
inline float far_clip = 1000000000;

inline Point camera_eye = Point(0, 0, 0);
inline Point camera_at = Point(0, 0, -1);
inline Vector camera_up = Vector(0, 1, 0);

inline std::vector<float> depth_buffer = {};

/* Transformations */

inline std::stack<XformArray> transform_stack = {};
inline std::stack<XformArray> normal_stack = {};

inline Xform current_transform = Xform();
inline Xform normal_transform = Xform();
inline Xform world_to_clip_transform = Xform();
inline Xform clip_to_device_transform = Xform();

/* Disks */

constexpr int LAT_SCALE_SIZE = DISK_SEGMENTS + 1;
constexpr int LON_SCALE_SIZE = DISK_SEGMENTS / 2 + 1;

inline float lat_sphere_scale[LAT_SCALE_SIZE] = { -1 };
inline float lon_sphere_scale[LON_SCALE_SIZE] = { -1 };
inline bool is_sphere_scale_set = false;

/* Line */

inline PointH current_line_point = PointH();
inline int current_line_code = 0b111111;
inline float current_line_bound_coords[6] = { 0, 0, 0, 0, 0, 0 };

/* Polygons */

inline Edge active_edge_list_head = Edge();

inline int edge_table_edges = 0;
inline Edge edge_table[MAX_POLYGON_EDGES] = {};

/* Lighting */

inline bool use_interpolated = true;
inline bool invert_normals = false;

inline AmbientLight ambient_light = { Color(1) };
inline std::vector<FarLight> far_lights = {};
inline std::vector<PointLight> point_lights = {};

inline bool vertex_has_color = false;
inline bool vertex_has_normal = false;
inline bool vertex_has_texture = false;

inline float ambient_coeff = 1.0;
inline float diffuse_coeff = 0.0;
inline float specular_coeff = 0.0;

inline float specular_exp = 10.0;

inline Color surface_color = Color(1);
inline Color specular_color = Color(1);

inline Vector polygon_normal = Vector();

inline PointA surface_point_values = PointA();
inline Color (*surface_shader_func)(const Color&) = Shader::matte;
};  // namespace g

#endif  // !GLOBALS
