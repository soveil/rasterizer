#ifndef COLOR
#define COLOR

#include <algorithm>
#include <string>

class Color {
 public:
  union {
    struct {
      float r;
      float g;
      float b;
    };

    struct {
      float array[3];
    };
  };

  Color() : Color(0, 0, 0) {}

  Color(float a) : Color(a, a, a) {}

  Color(float r, float g, float b);

  Color operator=(const float color[3]);
  Color operator*=(const Color& rhs);
  Color operator+=(const Color& rhs);
  Color operator-=(const Color& rhs);

  Color operator*=(const float rhs);
  Color operator/=(const float rhs);

  void clamp() {
    r = std::clamp(r, 0.0f, 1.0f);
    g = std::clamp(g, 0.0f, 1.0f);
    b = std::clamp(b, 0.0f, 1.0f);
  }

  Color clamped() const {
    Color c = *this;
    c.clamp();
    return c;
  }

  std::string to_string() const;

  friend Color operator+(Color lhs, const Color& rhs) { return lhs += rhs; }

  friend Color operator*(Color lhs, const Color& rhs) { return lhs *= rhs; }

  friend Color operator*(Color lhs, float rhs) { return lhs *= rhs; }

  friend Color operator/(Color lhs, float rhs) { return lhs /= rhs; }

  friend Color operator*(float lhs, Color rhs) { return rhs * lhs; }
};

#endif  // !COLOR
