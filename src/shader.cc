#include "shader.h"

#include <cmath>

#include "globals.h"

Color Shader::matte(const Color& c) {
  Color far = Color();
  Color point = Color();

  Vector N = get_normal();

  // far
  for (uint i = 0; i < g::far_lights.size(); ++i) {
    Vector L = g::far_lights[i].direction;
    Color Cf = g::far_lights[i].color;

    far += std::max(0.0f, N.dot(L)) * Cf;
  }
  far *= c * g::diffuse_coeff;

  // point
  for (uint i = 0; i < g::point_lights.size(); ++i) {
    Point surface = g::surface_point_values.world_point;
    Vector L = surface.vector_to(g::point_lights[i].position);
    float r2 = L.mag2();
    Color Cp = g::point_lights[i].color;

    L.normalize();

    point += std::max(0.0f, N.dot(L)) * Cp / r2;
  }
  point *= c * g::diffuse_coeff;

  return (Shader::ambient(c) + far + point).clamped();
}

Color Shader::metal(const Color& c) {
  Point surface = g::surface_point_values.world_point;
  Vector V = surface.vector_to(g::camera_eye).normalized();
  Vector N = get_normal();

  Color far_specular = Color();
  Color point_specular = Color();

  // far
  for (uint i = 0; i < g::far_lights.size(); ++i) {
    Vector L = g::far_lights[i].direction;

    if (N.dot(L) <= 0.0f) continue;  // don't create light on the back of surfaces

    Color Cf = g::far_lights[i].color;
    Vector R = L.reflected(N);

    far_specular += std::pow(std::max(0.0f, V.dot(R)), g::specular_exp) * Cf;
  }
  far_specular *= c * g::specular_coeff;

  // point
  for (uint i = 0; i < g::point_lights.size(); ++i) {
    Vector L = surface.vector_to(g::point_lights[i].position);

    if (N.dot(L) <= 0.0f) continue;  // don't create light on the back of surfaces

    Color Cp = g::point_lights[i].color;
    float r2 = L.mag2();
    L.normalize();
    Vector R = L.reflected(N);

    point_specular += std::pow(std::max(0.0f, V.dot(R)), g::specular_exp) * Cp / r2;
  }
  point_specular *= c * g::specular_coeff;

  return (Shader::ambient(c) + far_specular + point_specular).clamped();
}

Color Shader::plastic(const Color& c) {
  Point surface = g::surface_point_values.world_point;
  Vector V = surface.vector_to(g::camera_eye).normalized();
  Vector N = get_normal();

  Color far_diffuse = Color();
  Color far_specular = Color();
  Color point_diffuse = Color();
  Color point_specular = Color();

  // far
  for (uint i = 0; i < g::far_lights.size(); ++i) {
    Vector L = g::far_lights[i].direction;
    float NdotL = N.dot(L);

    if (NdotL <= 0.0f) continue;  // don't create light on the back of surfaces

    Color Cf = g::far_lights[i].color;
    Vector R = L.reflected(N);

    far_diffuse += NdotL * Cf;
    far_specular += std::pow(std::max(0.0f, V.dot(R)), g::specular_exp) * Cf;
  }
  far_diffuse *= c * g::diffuse_coeff;
  far_specular *= g::specular_color * g::specular_coeff;

  // point
  for (uint i = 0; i < g::point_lights.size(); ++i) {
    Vector L = surface.vector_to(g::point_lights[i].position);
    float r2 = L.mag2();

    L.normalize();
    float NdotL = N.dot(L);

    if (NdotL <= 0.0f) continue;  // don't create light on the back of surfaces

    Color Cp = g::point_lights[i].color;
    Vector R = L.reflected(N);

    point_diffuse += NdotL * Cp / r2;
    point_specular += std::pow(std::max(0.0f, V.dot(R)), g::specular_exp) * Cp / r2;
  }
  point_diffuse *= c * g::diffuse_coeff;
  point_specular *= g::specular_color * g::specular_coeff;

  return (Shader::ambient(c) + far_diffuse + far_specular + point_diffuse + point_specular).clamped();
}

Color Shader::ambient(const Color& c) {
  return g::ambient_light.color * g::ambient_coeff * c;
}

Vector Shader::get_normal() {
  return (g::use_interpolated && g::vertex_has_normal ? g::surface_point_values.normal_vector : g::polygon_normal).normalized();
}
