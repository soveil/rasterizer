#ifndef MATH
#define MATH

#include "types/color.h"

class Math {
 public:
  /*
   * MISCELLANEOUS
   */

  // Returns true if the color are equivalent within a margin of error
  static bool equal_colors(const Color& a, const Color& b);

  // Converts an array of floats to an array of ints after rounding
  // The rounded ints will be returned in round_arr
  // size should be the size of round_arr and float_arr
  static void round_to_int_array(const float* float_arr, int* round_arr, int size);

  // Converts an array of floats to an array of rounded floats
  // The rounded floats will be returned in round_arr
  // size should be the size of round_arr and float_arr
  // place should be the amount of decimal places to round to (default is 2)
  static void round_to_float_array(const float* float_arr, float* round_arr, int size, int place = 2);

  static float deg_to_rad(const float degree);
};

#endif  // MATH
