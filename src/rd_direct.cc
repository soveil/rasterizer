#include "rd_direct.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>

#include "globals.h"
#include "math.h"
#include "pipeline.h"
#include "rd_display.h"
#include "rd_error.h"
#include "shader.h"
#include "types/color.h"
#include "types/edge.h"
#include "types/point.h"
#include "types/pointa.h"
#include "types/pointh.h"
#include "types/vector.h"
#include "types/xform.h"

enum LINE_ACTION { MOVE, DRAW };

enum POLYGON_BOUNDARY { LEFT, RIGHT, TOP, BOTTOM, NEAR, FAR, END_BOUNDARY };

const int CLIP_PLANES = 6;

/**********************   Helper functions  ********************************/

/*
 * Pixel Writing
 */

int write_point(const Point& p, const Color& c) {
  int depth_i = p.x * display_ySize + p.y;
  if (p.z < g::depth_buffer[depth_i]) {
    rd_write_pixel(p.x, p.y, c.array);
    g::depth_buffer[depth_i] = p.z;
  }
  return RD_OK;
}

/*
 * Line Functions
 */

// Draws a clipped line from the global current_line_point to end_point
int draw_line(const PointH& start_point, const PointH& end_point, const Color& color) {
  Point start_device_point = Point(g::clip_to_device_transform * start_point);
  Point end_device_point = Point(g::clip_to_device_transform * end_point);

  int x_start = std::clamp(static_cast<int>(std::round(start_device_point.x)), 0, display_xSize - 1);
  int y_start = std::clamp(static_cast<int>(std::round(start_device_point.y)), 0, display_ySize - 1);
  float z_start = start_device_point.z;

  int x_end = std::clamp(static_cast<int>(std::round(end_device_point.x)), 0, display_xSize - 1);
  int y_end = std::clamp(static_cast<int>(std::round(end_device_point.y)), 0, display_ySize - 1);
  float z_end = end_device_point.z;

  int x_change = x_end - x_start;
  int y_change = y_end - y_start;
  float z_change = z_end - z_start;

  int num_steps = std::max(std::abs(x_change), std::abs(y_change));

  // if the line is a single point, write that point and exit
  if (num_steps == 0) {
    write_point(Point(x_start, y_start, z_start), color);
    return RD_OK;
  }

  float dx = float(x_change) / num_steps;
  float dy = float(y_change) / num_steps;
  float dz = float(z_change) / num_steps;

  start_device_point.x = x_start;
  start_device_point.y = y_start;
  start_device_point.z = z_start;

  for (int i = 0; i <= num_steps; ++i) {
    write_point(Point(std::round(x_start + dx * i), std::round(y_start + dy * i), z_start + dz * i), color);
  }

  return RD_OK;
}

int line_pipeline(const PointH& point, LINE_ACTION action, const Color& color) {
  PointH end_point = g::current_transform * point;
  end_point = g::world_to_clip_transform * end_point;

  float end_bound_coords[CLIP_PLANES] = { end_point.x, end_point.w - end_point.x, end_point.y, end_point.w - end_point.y, end_point.z, end_point.w - end_point.z };
  int end_code = 0;

  // Set boundary code for end point
  for (int i = 0; i < CLIP_PLANES; ++i)
    if (end_bound_coords[i] < 0) end_code |= (1 << i);

  if (action == DRAW) {
    // no trivial reject
    if ((g::current_line_code & end_code) == 0) {

      // trivial accept
      if ((g::current_line_code | end_code) == 0) {
        draw_line(g::current_line_point, end_point, color);
      }

      // nontrivial
      else {
        int clip = g::current_line_code | end_code;
        int mask = 1;
        float a = 0;
        float start_a = 0;
        float end_a = 1;
        bool rejected = false;

        // clip line to inside viewport
        for (int i = 0; i < CLIP_PLANES; ++i) {
          if ((clip & mask) != 0) {
            a = g::current_line_bound_coords[i] / (g::current_line_bound_coords[i] - end_bound_coords[i]);

            if ((g::current_line_code & mask) != 0)
              start_a = std::max(start_a, a);
            else
              end_a = std::min(end_a, a);

            // nontrivial reject
            if (end_a < start_a) {
              rejected = true;
              break;
            }
          }
          mask <<= 1;
        }

        if (!rejected) {
          PointH clipped_start = g::current_line_point;
          PointH clipped_end = end_point;

          if (g::current_line_code != 0) {
            clipped_start = g::current_line_point + start_a * (end_point - g::current_line_point);
          }

          if (end_code != 0) {
            clipped_end = g::current_line_point + end_a * (end_point - g::current_line_point);
          }
          draw_line(clipped_start, clipped_end, color);
        }
      }
    }
  }

  // Set current line point to end point (Also does everything move would do)
  g::current_line_point = end_point;
  g::current_line_code = end_code;

  for (int i = 0; i < CLIP_PLANES; ++i) {
    g::current_line_bound_coords[i] = end_bound_coords[i];
  }

  return RD_OK;
}

/*
 * Polygon Functions
 */

/* Scan Conversion */

bool build_edge_list(PointA* clipped_list, int n_vertex) {
  bool scanline_crossed = false;
  int i = 0;

  for (i = 0; i < n_vertex - 1; ++i) {
    // If v and v1 are on different scanlines
    if (std::ceil(clipped_list[i].pointh.get_y()) != std::ceil(clipped_list[i + 1].pointh.get_y())) {
      scanline_crossed = true;

      g::edge_table[g::edge_table_edges] = Edge(clipped_list[i], clipped_list[i + 1]);
      ++g::edge_table_edges;
    }
  }
  // Create an edge from the first to last vertex if they exist, unconnected, and on different scanlines
  if (i > 1 && std::ceil(clipped_list[0].pointh.get_y()) != std::ceil(clipped_list[i].pointh.get_y())) {
    // don't need to update scanline_crossed, they would've already
    g::edge_table[g::edge_table_edges] = Edge(clipped_list[0], clipped_list[i]);
    ++g::edge_table_edges;
  }
  return scanline_crossed;
}

// Removes edges that have ended and updates edge values
void update_active_edge_table(int scanline) {
  Edge* q = &g::active_edge_list_head;
  Edge* p = g::active_edge_list_head.next;

  while (p) {
    if (scanline == p->y_last) {
      p = p->next;
      remove_next_edge(q);
    }
    else {
      p->pointa += p->inc;
      q = p;
      p = p->next;
    }
  }
}

// Fills edges in active edge table at scanline
void fill_scan(int scanline) {
  Edge* e1 = g::active_edge_list_head.next;
  Edge* e2 = nullptr;
  while (e1 != nullptr && e1->next != nullptr) {
    e2 = e1->next;

    // skip edges with the same x value
    if (int(e1->pointa.pointh.x) == int(e2->pointa.pointh.x)) {
      e1 = e2->next;
      continue;
    }

    float dx = e2->pointa.pointh.x - e1->pointa.pointh.x;
    PointA inc = (e2->pointa - e1->pointa) / dx;

    float factor = std::ceil(e1->pointa.pointh.x) - e1->pointa.pointh.x;
    PointA value = e1->pointa + factor * inc;
    int end_x = std::ceil(e2->pointa.pointh.x);

    while (value.pointh.x < end_x) {
      g::surface_point_values = value / value.k;
      write_point(Point(value.pointh.x, scanline, value.pointh.z), g::surface_shader_func(g::surface_point_values.color));
      value += inc;
    }
    e1 = e2->next;
  }
}

int scan_convert(PointA* clipped_list, int n_vertex) {
  if (!build_edge_list(clipped_list, n_vertex)) {
    return RD_OK;  // No edges crossed
  }

  g::active_edge_list_head.next = nullptr;

  for (int y = 0; y < display_ySize; ++y) {
    // Add edges starting on this scanline to the active edge table
    for (int i = 0; i < g::edge_table_edges; ++i) {
      if (g::edge_table[i].y_first == y) {
        insert_edge(&g::active_edge_list_head, &g::edge_table[i]);
      }
    }

    if (g::active_edge_list_head.next != nullptr) {
      fill_scan(y);
      update_active_edge_table(y);
      sort_table(&g::active_edge_list_head);
    }
  }

  return RD_OK;
}

bool inside(PointA& p, POLYGON_BOUNDARY bound) {
  switch (bound) {
    case LEFT:
      return p.pointh.x >= 0;
    case RIGHT:
      return p.pointh.w - p.pointh.x >= 0;
    case TOP:
      return p.pointh.y >= 0;
    case BOTTOM:
      return p.pointh.w - p.pointh.y >= 0;
    case NEAR:
      return p.pointh.z >= 0;
    case FAR:
      return p.pointh.w - p.pointh.z >= 0;
    case END_BOUNDARY:
      break;
  }
  return false;
}

PointA intersect(PointA& p1, PointA& p2, POLYGON_BOUNDARY bound) {
  float a = 0;
  // a = g::current_line_bound_coords[i] / (g::current_line_bound_coords[i] - end_bound_coords[i]);

  switch (bound) {
    case LEFT:
      a = p1.pointh.x / (p1.pointh.x - p2.pointh.x);
      break;
    case RIGHT:
      a = (p1.pointh.x - p1.pointh.w) / ((p1.pointh.x - p1.pointh.w) - (p2.pointh.x - p2.pointh.w));
      break;
    case TOP:
      a = p1.pointh.y / (p1.pointh.y - p2.pointh.y);
      break;
    case BOTTOM:
      a = (p1.pointh.y - p1.pointh.w) / ((p1.pointh.y - p1.pointh.w) - (p2.pointh.y - p2.pointh.w));
      break;
    case NEAR:
      a = p1.pointh.z / (p1.pointh.z - p2.pointh.z);
      break;
    case FAR:
      a = (p1.pointh.z - p1.pointh.w) / ((p1.pointh.z - p1.pointh.w) - (p2.pointh.z - p2.pointh.w));
      break;
    case END_BOUNDARY:
      break;
  }

  return p1 + a * (p2 - p1);
}

void clip_vertex(PointA& vertex, PointA* clipped_list, POLYGON_BOUNDARY bound, PointA* first_vertices, PointA* last_vertices, bool seen_bounds[], int& n_clipped) {
  if (!seen_bounds[bound]) {
    first_vertices[bound] = vertex;
    last_vertices[bound] = vertex;
    seen_bounds[bound] = true;
  }
  else {
    // if created edge crosses the boundary
    if (inside(vertex, bound) != inside(last_vertices[bound], bound)) {
      PointA ipt = intersect(vertex, last_vertices[bound], bound);
      POLYGON_BOUNDARY next_bound = POLYGON_BOUNDARY(bound + 1);
      if (next_bound == END_BOUNDARY)
        clipped_list[n_clipped++] = ipt;
      else
        clip_vertex(ipt, clipped_list, next_bound, first_vertices, last_vertices, seen_bounds, n_clipped);
    }
  }
  last_vertices[bound] = vertex;
  if (inside(vertex, bound)) {
    POLYGON_BOUNDARY next_bound = POLYGON_BOUNDARY(bound + 1);
    if (next_bound == END_BOUNDARY)
      clipped_list[n_clipped++] = vertex;
    else
      clip_vertex(vertex, clipped_list, next_bound, first_vertices, last_vertices, seen_bounds, n_clipped);
  }
}

void clip_last_edge(PointA* clipped_list, PointA* first_vertices, PointA* last_vertices, bool seen_bounds[], int& n_clipped) {
  for (int b = 0; b != END_BOUNDARY; ++b) {
    if (seen_bounds[b] && inside(first_vertices[b], POLYGON_BOUNDARY(b)) != inside(last_vertices[b], POLYGON_BOUNDARY(b))) {
      PointA ipt = intersect(first_vertices[b], last_vertices[b], POLYGON_BOUNDARY(b));
      POLYGON_BOUNDARY next_bound = POLYGON_BOUNDARY(b + 1);
      if (next_bound == END_BOUNDARY)
        clipped_list[n_clipped++] = ipt;
      else
        clip_vertex(ipt, clipped_list, next_bound, first_vertices, last_vertices, seen_bounds, n_clipped);
    }
  }
}

// Returns the final number of vertices in the polygon
int poly_clip(int n_vertex, PointA* vertex_list, PointA* clipped_list) {
  PointA first_vertices[CLIP_PLANES] = {};
  PointA last_vertices[CLIP_PLANES] = {};
  int n_clipped = 0;
  bool seen_bounds[CLIP_PLANES] = { false, false, false, false, false, false };

  for (int i = 0; i < n_vertex; ++i) {
    clip_vertex(vertex_list[i], clipped_list, LEFT, first_vertices, last_vertices, seen_bounds, n_clipped);
  }

  // clip the final edge if needed
  if (n_vertex > 2) {
    clip_last_edge(clipped_list, first_vertices, last_vertices, seen_bounds, n_clipped);
  }
  return n_clipped;
}

int poly_pipeline(PointA point, LINE_ACTION action) {
  static PointA vertex_list[g::MAX_POLYGON_VERTICES];
  static PointA clipped_list[g::MAX_POLYGON_VERTICES];
  static int n_vertex = 0;

  // transform point from object to world to clip coordinates
  point.pointh = g::current_transform * point.pointh;
  point.world_point = point.pointh;  // save world coordinates for later

  point.pointh = g::world_to_clip_transform * point.pointh;

  // transform normal vector from object to world coordinates
  point.normal_vector = g::normal_transform * point.normal_vector;

  if (n_vertex >= g::MAX_POLYGON_VERTICES) {
    return RD_INPUT_ILLEGAL_VERTEX_INDEX;
  }

  vertex_list[n_vertex] = point;
  ++n_vertex;

  // Go to the next vertex
  if (action == MOVE) return RD_OK;

  if ((n_vertex = poly_clip(n_vertex, vertex_list, clipped_list)) >= 3) {
    for (int i = 0; i < n_vertex; ++i) {
      clipped_list[i].normalize();
      clipped_list[n_vertex].k = 1;
    }

    for (int i = 0; i < n_vertex; ++i) {
      clipped_list[i].pointh = g::clip_to_device_transform * clipped_list[i].pointh;
    }
    scan_convert(clipped_list, n_vertex);
  }

  for (int i = 0; i < g::edge_table_edges; ++i) {
    g::edge_table[i] = Edge();
  }
  g::edge_table_edges = 0;
  n_vertex = 0;
  return RD_OK;
}

/*
 * Fill Functions
 */

// Find the range of pixels originating from (x,y) that should be filled on y
// Sets [x_start, y_start)
int find_span(const int x, const int y, const Color& seed_color, int& x_start, int& x_end) {
  Color point_color;

  // Decrease x_start until color mismatch or at 0
  for (x_start = x; x_start >= 0; --x_start) {
    rd_read_pixel(x_start, y, point_color.array);
    if (!Math::equal_colors(point_color, seed_color)) {
      break;
    }
  }
  ++x_start;

  // Increase x_end until color mismatch or at xSize
  for (x_end = x; x_end < display_xSize; ++x_end) {
    rd_read_pixel(x_end, y, point_color.array);
    if (!Math::equal_colors(point_color, seed_color)) {
      break;
    }
  }
  return RD_OK;
}

// Fill the pixels [x_start, x_end)
int fill_span(const int x_start, const int x_end, const int y) {
  for (int x = x_start; x < x_end; ++x) {
    write_point(Point(x, y, 0), g::fg_color);
  }
  return RD_OK;
}

// Recursively fill every pixel adjacent to (x, y) with the same color as seed_color
int recursive_fill(const int x, const int y, const Color& seed_color) {
  if (y < 0 || y >= display_ySize) {
    return RD_OK;
  }
  int x_start;
  int x_end;
  find_span(x, y, seed_color, x_start, x_end);
  fill_span(x_start, x_end, y);
  for (int x_it = x_start; x_it < x_end; ++x_it) {
    recursive_fill(x_it, y - 1, seed_color);
    recursive_fill(x_it, y + 1, seed_color);
  }
  return RD_OK;
}

/**********************   General functions  *******************************/

int REDirect::rd_display([[maybe_unused]] const string& name, [[maybe_unused]] const string& type, [[maybe_unused]] const string& mode) {
  return RD_OK;
}

int REDirect::rd_format([[maybe_unused]] int xresolution, [[maybe_unused]] int yresolution) {
  return RD_OK;
}

int REDirect::rd_world_begin(void) {
  rd_disp_init_frame(g::frame_number);

  if (g::depth_buffer.empty()) {
    g::depth_buffer.resize(display_xSize * display_ySize, INFINITY);
  }
  else {
    g::depth_buffer.assign(display_xSize * display_ySize, INFINITY);
  }

  Xform world_to_camera = Pipeline::world_to_camera();
  Xform camera_to_clip = Pipeline::camera_to_clip();

  g::current_transform = Xform::identity();
  g::normal_transform = Xform::identity();
  g::world_to_clip_transform = camera_to_clip * world_to_camera;
  g::clip_to_device_transform = Pipeline::clip_to_device();

  return RD_OK;
}

int REDirect::rd_world_end(void) {
  rd_disp_end_frame();
  return RD_OK;
}

int REDirect::rd_frame_begin(int frame_no) {
  g::frame_number = frame_no;
  return RD_OK;
}

int REDirect::rd_frame_end(void) {
  rd_disp_end_frame();
  g::ambient_light = { Color(1) };
  g::far_lights = {};
  g::point_lights = {};
  return RD_OK;
}

int REDirect::rd_option_bool(const string& name, bool flag) {
  if (name == "Interpolate")
    g::use_interpolated = flag;
  else if (name == "InvertNormals")
    g::invert_normals = flag;
  return RD_OK;
}

/**********************   Camera  ******************************************/

int REDirect::rd_camera_eye(const float eyepoint[3]) {
  g::camera_eye = Point(eyepoint[0], eyepoint[1], eyepoint[2]);
  return RD_OK;
}

int REDirect::rd_camera_at(const float atpoint[3]) {
  g::camera_at = Point(atpoint[0], atpoint[1], atpoint[2]);
  return RD_OK;
}

int REDirect::rd_camera_up(const float up[3]) {
  g::camera_up = Vector(up[0], up[1], up[2]);
  return RD_OK;
}

int REDirect::rd_camera_fov(float fov) {
  g::camera_fov = fov;
  return RD_OK;
}

int REDirect::rd_clipping(float znear, float zfar) {
  g::near_clip = znear;
  g::far_clip = zfar;
  return RD_OK;
}

/**********************   Transformations **********************************/

int REDirect::rd_translate(const float offset[3]) {
  g::current_transform.translate(offset[0], offset[1], offset[2]);
  return RD_OK;
}

int REDirect::rd_scale(const float scale_factor[3]) {
  g::current_transform.scale(scale_factor[0], scale_factor[1], scale_factor[2]);
  g::normal_transform.scale(1 / scale_factor[0], 1 / scale_factor[1], 1 / scale_factor[2]);
  return RD_OK;
}

int REDirect::rd_rotate_xy(float angle) {
  g::current_transform.rotate_xy(angle);
  g::normal_transform.rotate_xy(angle);
  return RD_OK;
}

int REDirect::rd_rotate_yz(float angle) {
  g::current_transform.rotate_yz(angle);
  g::normal_transform.rotate_yz(angle);
  return RD_OK;
}

int REDirect::rd_rotate_zx(float angle) {
  g::current_transform.rotate_zx(angle);
  g::normal_transform.rotate_zx(angle);
  return RD_OK;
}

int REDirect::rd_xform_push(void) {
  g::transform_stack.push(g::current_transform.array);
  g::normal_stack.push(g::normal_transform.array);
  return RD_OK;
}

int REDirect::rd_xform_pop(void) {
  if (g::transform_stack.empty()) {
    return RD_INPUT_TRANSFORM_STACK_UNDERFLOW;
  }
  if (g::normal_stack.empty()) {
    return RD_INPUT_TRANSFORM_STACK_UNDERFLOW;
  }

  g::current_transform = g::transform_stack.top();
  g::transform_stack.pop();

  g::normal_transform = g::normal_stack.top();
  g::normal_stack.pop();
  return RD_OK;
}

/**********************   Geometric Objects  *******************************/

Vector vertex_normal(const Vector& normal) {
  if (g::invert_normals) return -normal;
  return normal;
}

void set_polygon_normal(Vector normal) {
  if (g::invert_normals) normal = -normal;
  g::polygon_normal = g::normal_transform * normal;
}

void set_polygon_normal(const PointH& p1, const PointH& p2, const PointH& p3) {
  Vector d1 = p1.vector_to(p2);
  Vector d2 = p1.vector_to(p3);
  set_polygon_normal(d1.cross(d2));
}

void set_polygon_normal(const PointH& p1, const PointH& p2, const PointH& p3, const PointH& p4) {
  Vector d1 = p1.vector_to(p3);
  Vector d2 = p2.vector_to(p4);
  set_polygon_normal(d1.cross(d2));
}

int REDirect::rd_circle(const float center[3], float radius) {
  int r = std::round(radius);
  int p = 1 - r;
  int y = r;

  for (int x = 0; y >= x;) {
    // Draw on each octet of the circle
    write_point(Point(x + center[0], y + center[1], center[2]), g::fg_color);
    write_point(Point(x + center[0], -y + center[1], center[2]), g::fg_color);
    write_point(Point(-x + center[0], y + center[1], center[2]), g::fg_color);
    write_point(Point(-x + center[0], -y + center[1], center[2]), g::fg_color);

    write_point(Point(y + center[0], x + center[1], center[2]), g::fg_color);
    write_point(Point(y + center[0], -x + center[1], center[2]), g::fg_color);
    write_point(Point(-y + center[0], x + center[1], center[2]), g::fg_color);
    write_point(Point(-y + center[0], -x + center[1], center[2]), g::fg_color);

    // Decide whether to advance y or not
    ++x;
    if (p >= 0) {  // On or outside the circle
      --y;
      p += 2 * x - 2 * y + 1;
    }
    else {  // Inside the circle
      p += 2 * x + 1;
    }
  }
  return RD_OK;
}

int REDirect::rd_cone(float height, float radius, [[maybe_unused]] float thetamax) {
  static_assert(g::DISK_SEGMENTS > 2, "DISK_SEGMENTS must be greater than 2");
  float rad_segment_diff = M_PI * 2 / g::DISK_SEGMENTS;
  PointA tip = PointA(0, 0, height, g::surface_color, vertex_normal(Vector(0, 0, 1)));

  // Draw around the disk
  for (int i = g::DISK_SEGMENTS - 1; i >= 0; --i) {
    float angle1 = (i % g::DISK_SEGMENTS) * rad_segment_diff;
    float x1 = radius * std::sin(angle1);
    float y1 = radius * std::cos(angle1);

    float angle2 = ((i - 1) % g::DISK_SEGMENTS) * rad_segment_diff;
    float x2 = radius * std::sin(angle2);
    float y2 = radius * std::cos(angle2);

    PointA point1 = PointA(x1, y1, 0, g::surface_color, vertex_normal(Vector(x1, y1, 0)));
    PointA point2 = PointA(x2, y2, 0, g::surface_color, vertex_normal(Vector(x2, y2, 0)));

    set_polygon_normal(point1.pointh, point2.pointh, tip.pointh);

    poly_pipeline(point1, MOVE);
    poly_pipeline(point2, MOVE);
    poly_pipeline(tip, MOVE);
    poly_pipeline(tip, DRAW);
  }

  return RD_OK;
}

int REDirect::rd_cube() {
  g::vertex_has_color = false;
  g::vertex_has_normal = true;
  g::vertex_has_texture = false;

  Vector normal;

  PointA right_top_front = PointA(1, 1, 1, g::surface_color);
  PointA right_top_back = PointA(1, 1, -1, g::surface_color);
  PointA right_bottom_front = PointA(1, -1, 1, g::surface_color);
  PointA right_bottom_back = PointA(1, -1, -1, g::surface_color);
  PointA left_top_front = PointA(-1, 1, 1, g::surface_color);
  PointA left_top_back = PointA(-1, 1, -1, g::surface_color);
  PointA left_bottom_front = PointA(-1, -1, 1, g::surface_color);
  PointA left_bottom_back = PointA(-1, -1, -1, g::surface_color);

  // Right face
  normal = vertex_normal(Vector(1, 0, 0));
  right_top_front.normal_vector = normal;
  right_top_back.normal_vector = normal;
  right_bottom_front.normal_vector = normal;
  right_bottom_back.normal_vector = normal;

  set_polygon_normal(Vector(1, 0, 0));

  poly_pipeline(right_top_front, MOVE);
  poly_pipeline(right_bottom_front, MOVE);
  poly_pipeline(right_bottom_back, MOVE);
  poly_pipeline(right_top_back, DRAW);

  // Front face
  normal = vertex_normal(Vector(0, 0, 1));
  left_top_front.normal_vector = normal;
  left_bottom_front.normal_vector = normal;
  right_bottom_front.normal_vector = normal;
  right_top_front.normal_vector = normal;

  set_polygon_normal(Vector(1, 0, 1));

  poly_pipeline(left_top_front, MOVE);
  poly_pipeline(left_bottom_front, MOVE);
  poly_pipeline(right_bottom_front, MOVE);
  poly_pipeline(right_top_front, DRAW);

  // Top face
  normal = vertex_normal(Vector(0, 1, 0));
  right_top_back.normal_vector = normal;
  left_top_back.normal_vector = normal;
  left_top_front.normal_vector = normal;
  right_top_front.normal_vector = normal;

  set_polygon_normal(Vector(0, 1, 0));

  poly_pipeline(right_top_back, MOVE);
  poly_pipeline(left_top_back, MOVE);
  poly_pipeline(left_top_front, MOVE);
  poly_pipeline(right_top_front, DRAW);

  // Bottom face
  normal = vertex_normal(Vector(0, -1, 0));
  right_bottom_back.normal_vector = normal;
  right_bottom_front.normal_vector = normal;
  left_bottom_front.normal_vector = normal;
  left_bottom_back.normal_vector = normal;

  set_polygon_normal(Vector(0, -1, 0));

  poly_pipeline(right_bottom_back, MOVE);
  poly_pipeline(right_bottom_front, MOVE);
  poly_pipeline(left_bottom_front, MOVE);
  poly_pipeline(left_bottom_back, DRAW);

  // Back face
  normal = vertex_normal(Vector(0, 0, -1));
  left_top_back.normal_vector = normal;
  right_top_back.normal_vector = normal;
  right_bottom_back.normal_vector = normal;
  left_bottom_back.normal_vector = normal;

  set_polygon_normal(Vector(0, 0, -1));

  poly_pipeline(left_top_back, MOVE);
  poly_pipeline(right_top_back, MOVE);
  poly_pipeline(right_bottom_back, MOVE);
  poly_pipeline(left_bottom_back, DRAW);

  // Left face
  normal = vertex_normal(Vector(-1, 0, 0));
  left_bottom_front.normal_vector = normal;
  left_top_front.normal_vector = normal;
  left_top_back.normal_vector = normal;
  left_bottom_back.normal_vector = normal;

  set_polygon_normal(Vector(-1, 0, 0));

  poly_pipeline(left_bottom_front, MOVE);
  poly_pipeline(left_top_front, MOVE);
  poly_pipeline(left_top_back, MOVE);
  poly_pipeline(left_bottom_back, DRAW);

  return RD_OK;
}

int REDirect::rd_cylinder(float radius, float zmin, float zmax, [[maybe_unused]] float thetamax) {
  g::vertex_has_color = false;
  g::vertex_has_normal = true;
  g::vertex_has_texture = false;

  static_assert(g::DISK_SEGMENTS > 2, "DISK_SEGMENTS must be greater than 2");
  float rad_segment_diff = M_PI * 2 / g::DISK_SEGMENTS;

  // Draw around the disk
  for (int i = g::DISK_SEGMENTS - 1; i >= 0; --i) {
    float first_angle = i * rad_segment_diff;
    float second_angle = (i - 1) * rad_segment_diff;

    float first_angle_x = radius * std::sin(first_angle);
    float first_angle_y = radius * std::cos(first_angle);
    float second_angle_x = radius * std::sin(second_angle);
    float second_angle_y = radius * std::cos(second_angle);

    Vector normal1 = vertex_normal(Vector(first_angle_x, first_angle_y, 0));
    Vector normal2 = vertex_normal(Vector(second_angle_x, second_angle_y, 0));

    PointA p1 = PointA(first_angle_x, first_angle_y, zmin, g::surface_color, normal1);
    PointA p2 = PointA(second_angle_x, second_angle_y, zmin, g::surface_color, normal2);
    PointA p3 = PointA(second_angle_x, second_angle_y, zmax, g::surface_color, normal2);

    set_polygon_normal(p1.pointh, p2.pointh, p3.pointh);

    poly_pipeline(p1, MOVE);
    poly_pipeline(p2, MOVE);
    poly_pipeline(p3, MOVE);
    poly_pipeline(PointA(first_angle_x, first_angle_y, zmax, g::surface_color, normal1), DRAW);
  }

  return RD_OK;
}

int REDirect::rd_disk(float height, float radius, [[maybe_unused]] float theta) {
  g::vertex_has_color = false;
  g::vertex_has_normal = true;
  g::vertex_has_texture = false;

  static_assert(g::DISK_SEGMENTS > 2, "DISK_SEGMENTS must be greater than 2");
  float rad_segment_diff = M_PI * 2 / g::DISK_SEGMENTS;

  Vector normal = vertex_normal(Vector(0, 0, 1));

  // Draw around the disk
  for (int i = g::DISK_SEGMENTS - 1; i > 0; --i) {
    float angle = i * rad_segment_diff;

    poly_pipeline(PointA(radius * std::sin(angle), radius * std::cos(angle), height, g::surface_color, normal), MOVE);
  }

  set_polygon_normal(Vector(0, 0, 1));
  poly_pipeline(PointA(0, radius, height, g::surface_color, normal), DRAW);

  return RD_OK;
}

int REDirect::rd_line(const float start[3], const float end[3]) {
  PointH start_point = PointH(start[0], start[1], start[2]);

  PointH end_point = PointH(end[0], end[1], end[2]);

  line_pipeline(start_point, MOVE, g::fg_color);
  line_pipeline(end_point, DRAW, g::fg_color);

  return RD_OK;
}

int REDirect::rd_point(const float p[3]) {
  // Run point through graphics pipeline
  PointH pointh = PointH(p[0], p[1], p[2]);
  pointh = g::current_transform * pointh;
  pointh = g::world_to_clip_transform * pointh;

  // clip point if x, y, or z is not between 0-1
  if (pointh.get_x() < 0 || pointh.get_x() > 1 || pointh.get_y() < 0 || pointh.get_y() > 1 || pointh.get_z() < 0 || pointh.get_z() > 1) {
    return RD_OK;
  }

  pointh = g::clip_to_device_transform * pointh;

  write_point(Point(pointh), g::fg_color);
  return RD_OK;
}

int REDirect::rd_polyset([[maybe_unused]] const string& vertex_type, [[maybe_unused]] int nvertex, const vector<float>& vertex, int nface, const vector<int>& face) {
  g::vertex_has_color = false;
  g::vertex_has_normal = false;
  g::vertex_has_texture = false;

  std::vector<PointA> vertices = {};

  int face_vertex = 0;
  int face_index = 0;

  while (face_index < nface) {
    if (face[face_vertex] >= 0) {
      vertices.push_back(PointA(&vertex[face[face_vertex] * 3], g::surface_color));
    }

    // end of the face
    else if (face[face_vertex] == -1) {
      // calculate polygon normal
      if (vertices.size() >= 4) {
        set_polygon_normal(vertices[0].pointh, vertices[1].pointh, vertices[2].pointh, vertices[3].pointh);
      }
      else if (vertices.size() == 3) {
        set_polygon_normal(vertices[0].pointh, vertices[1].pointh, vertices[2].pointh);
      }
      else {
        std::cerr << "NOT CALCULATING NORMAL" << std::endl;
      }

      // draw polygon
      uint i;
      for (i = 0; i < vertices.size() - 1; ++i) {
        poly_pipeline(vertices[i], MOVE);
      }
      poly_pipeline(vertices[i], DRAW);
      vertices.clear();
      ++face_index;
    }

    else
      return RD_INPUT_ILLEGAL_VERTEX_INDEX;

    ++face_vertex;
  }
  return RD_OK;
}

int REDirect::rd_pointset([[maybe_unused]] const string& vertex_type, int nvertex, const vector<float>& vertex) {
  for (int i = 0; i < nvertex; ++i) {
    rd_point(&vertex[i]);
  }
  return RD_OK;
}

PointA sphere_compute_point(float radius, float lon, float lat) {
  float R = radius * std::cos(lon);

  float x = R * std::cos(lat);
  float y = R * std::sin(lat);
  float z = radius * std::sin(lon);

  return PointA(x, y, z, g::surface_color, vertex_normal(Vector(x, y, z)));
}

int REDirect::rd_sphere(float radius, [[maybe_unused]] float zmin, [[maybe_unused]] float zmax, [[maybe_unused]] float thetamax) {
  g::vertex_has_color = false;
  g::vertex_has_normal = true;
  g::vertex_has_texture = false;

  // Populate lon and lat lookup arrays if they are not populated
  if (!g::is_sphere_scale_set) {
    float lat = 0;
    float lon = -M_PI / 2;
    for (int i = 0; i < g::LAT_SCALE_SIZE; ++i) {
      g::lat_sphere_scale[i] = lat;
      lat += M_PI * 2 / g::DISK_SEGMENTS;
    }

    for (int i = 0; i < g::LON_SCALE_SIZE; ++i) {
      g::lon_sphere_scale[i] = lon;
      lon += M_PI * 2 / g::DISK_SEGMENTS;
    }

    g::is_sphere_scale_set = true;
  }

  for (int lon = 0; lon < g::LON_SCALE_SIZE - 1; ++lon) {
    for (int lat = 0; lat < g::LAT_SCALE_SIZE - 1; ++lat) {
      PointA p1 = sphere_compute_point(radius, g::lon_sphere_scale[lon], g::lat_sphere_scale[lat]);
      poly_pipeline(p1, MOVE);

      PointA p2 = sphere_compute_point(radius, g::lon_sphere_scale[lon], g::lat_sphere_scale[lat + 1]);
      poly_pipeline(p2, MOVE);

      PointA p3 = sphere_compute_point(radius, g::lon_sphere_scale[lon + 1], g::lat_sphere_scale[lat + 1]);
      poly_pipeline(p3, MOVE);

      PointA p4 = sphere_compute_point(radius, g::lon_sphere_scale[lon + 1], g::lat_sphere_scale[lat]);

      set_polygon_normal(p1.pointh, p2.pointh, p3.pointh, p4.pointh);
      poly_pipeline(p4, DRAW);
    }
  }

  return RD_OK;
}

/********************  Lighting & Shading  ***************************/

int REDirect::rd_background(const float color[]) {
  g::bg_color = color;
  rd_set_background(color);
  return RD_OK;
}

int REDirect::rd_color(const float color[]) {
  g::fg_color = color;
  g::surface_color = color;
  return RD_OK;
}

int REDirect::rd_fill(const float seed_point[3]) {
  int int_seed_point[3];
  Math::round_to_int_array(seed_point, int_seed_point, 3);
  Color seed_color;
  rd_read_pixel(int_seed_point[0], int_seed_point[1], seed_color.array);

  // Prevent filling same points forever
  if (Math::equal_colors(seed_color, g::fg_color)) {
    return RD_OK;
  }

  recursive_fill(int_seed_point[0], int_seed_point[1], seed_color);
  return RD_OK;
}

int REDirect::rd_surface(const string& shader_type) {
  if (shader_type == "matte") {
    g::surface_shader_func = Shader::matte;
  }
  else if (shader_type == "metal") {
    g::surface_shader_func = Shader::metal;
  }
  else if (shader_type == "plastic") {
    g::surface_shader_func = Shader::plastic;
  }
  else {
    return RD_INPUT_UNKNOWN_SURFACE_TYPE;
  }
  return RD_OK;
}

int REDirect::rd_ambient_light(const float color[], float intensity) {
  g::ambient_light = { { color[0] * intensity, color[1] * intensity, color[2] * intensity } };
  return RD_OK;
}

int REDirect::rd_far_light(const float dir[3], const float color[], float intensity) {
  // far light direction vector is reversed
  g::far_lights.push_back({ { color[0] * intensity, color[1] * intensity, color[2] * intensity }, Vector(-dir[0], -dir[1], -dir[2]).normalized() });
  return RD_OK;
}

int REDirect::rd_point_light(const float pos[3], const float color[], float intensity) {
  g::point_lights.push_back({ { color[0] * intensity, color[1] * intensity, color[2] * intensity }, Point(pos[0], pos[1], pos[2]) });
  return RD_OK;
}

int REDirect::rd_k_ambient(float Ka) {
  g::ambient_coeff = Ka;
  return RD_OK;
}

int REDirect::rd_k_diffuse(float Kd) {
  g::diffuse_coeff = Kd;
  return RD_OK;
}

int REDirect::rd_k_specular(float Ks) {
  g::specular_coeff = Ks;
  return RD_OK;
}

int REDirect::rd_specular_color(const float color[], int exponent) {
  g::specular_color = color;
  g::specular_exp = exponent;
  return RD_OK;
}
