#ifndef PIPELINE
#define PIPELINE

#include "types/xform.h"

class Pipeline {
 public:
  static Xform world_to_camera();
  static Xform camera_to_clip();
  static Xform clip_to_device();
};

#endif  // ! PIPELINE
