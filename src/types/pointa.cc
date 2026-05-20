#include "pointa.h"

#include "point.h"
#include "vector.h"

PointA::PointA(const float x, const float y, const float z, const float w) {
  pointh = PointH(x, y, z, w);
  color = Color();
  normal_vector = Vector();
  k = 1;
  s = 0;
  t = 0;
  world_point = Point();
}

PointA::PointA(const PointH pointh, const Color color, const Vector normal_vector, const float k, const float s, const float t, const Point world_point) {
  this->pointh = pointh;
  this->color = color;
  this->normal_vector = normal_vector;
  this->k = k;
  this->s = s;
  this->t = t;
  this->world_point = world_point;
}

PointA& PointA::operator+=(const PointA& other) {
  pointh += other.pointh;
  color += other.color;
  normal_vector += other.normal_vector;
  k += other.k;
  s += other.s;
  t += other.t;
  world_point += other.world_point;
  return *this;
}

PointA& PointA::operator-=(const PointA& other) {
  pointh -= other.pointh;
  color -= other.color;
  normal_vector -= other.normal_vector;
  k -= other.k;
  s -= other.s;
  t -= other.t;
  world_point -= other.world_point;
  return *this;
}

PointA& PointA::operator*=(const float other) {
  pointh *= other;
  color *= other;
  normal_vector *= other;
  k *= other;
  s *= other;
  t *= other;
  world_point *= other;
  return *this;
}

PointA& PointA::operator/=(const float other) {
  pointh /= other;
  color /= other;
  normal_vector /= other;
  k /= other;
  s /= other;
  t /= other;
  world_point /= other;
  return *this;
}

void PointA::normalize() {
  *this /= pointh.w;
  pointh.w = 1;
}
