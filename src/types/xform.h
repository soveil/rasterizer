#ifndef XFORM
#define XFORM

#include <array>
#include <string>

constexpr int XformRows = 4;
constexpr int XformCols = 4;

using XformArray = std::array<float, XformRows * XformCols>;

class Xform {
 public:
  XformArray array;

  Xform();

  Xform(Xform& other);

  Xform(XformArray array);

  /*
   * Access operator (row, col)
   */
  float& operator()(int row, int col);

  Xform& operator=(Xform other);

  Xform operator*=(const XformArray);

  /*
   * Constant access operator (row, col)
   */
  float get(int row, int col) const;

  /*
   * Returns an identity matrix
   */
  static Xform identity();

  /*
   * Translates xform by the arguments
   */
  void translate(float tx, float ty, float tz);

  /*
   * Scales xform by the arguments
   */
  void scale(float sx, float sy, float sz);

  /*
   * Rotates x towards y by angle degrees
   */
  void rotate_xy(float angle);

  /*
   * Rotates y towards z by angle degrees
   */
  void rotate_yz(float angle);

  /*
   * Rotates z towards x by angle degrees
   */
  void rotate_zx(float angle);

  std::string to_string() const;

  friend Xform operator*(Xform lhs, const XformArray rhs) { return lhs *= rhs; }

  /*
   * Matrix multiplication
   * Assumes matrices can be multiplied together; namely lhs rows = rhs columns
   */
  friend Xform operator*(const Xform& lhs, const Xform& rhs) {
    Xform res;

    // Iterate through result matrix
    for (int r = 0; r < XformRows; ++r) {
      for (int c = 0; c < XformCols; ++c) {

        // Sum matrix cells together into result matrix
        for (int mut_cell = 0; mut_cell < XformRows; ++mut_cell) {
          res(r, c) += lhs.get(r, mut_cell) * rhs.get(mut_cell, c);
        }
      }
    }

    return res;
  }
};

#endif  // !XFORM
