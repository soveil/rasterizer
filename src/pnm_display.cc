#include "pnm_display.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "rd_display.h"
#include "rd_error.h"


// Access color value in image buffer at (x,y) point for rgb value 0,1, or 2
#define IMAGE_BUFFER(x, y, rgb) image_buffer[x * display_ySize * 3 + y * 3 + rgb]

std::vector<float> image_buffer;  // stores 3 floats per pixel (x major)
int pnm_frame_number = 0;

const float ZERO = 0.0;
const float ONE = 1.0;

float pnm_bg_color[3];

int pnm_init_display(void) {
  image_buffer.resize(display_xSize * display_ySize * 3);
  return RD_OK;
}

int pnm_end_display(void) {
  return RD_OK;
}

int pnm_init_frame(int frame) {
  pnm_clear();
  pnm_frame_number = frame;
  std::cout << "\rProcessing Frame: " << pnm_frame_number << std::flush;
  return RD_OK;
}

int pnm_end_frame(void) {
  std::ostringstream filename_oss;
  filename_oss << display_name << '_' << std::setw(3) << std::setfill('0') << pnm_frame_number << ".ppm";
  std::ofstream ppm;

  ppm.open(filename_oss.str(), std::ios::binary);

  // Print header
  ppm << "P6\n" << display_xSize << " " << display_ySize << "\n";
  ppm << "255\n";

  for (int y = 0; y < display_ySize; ++y) {
    for (int x = 0; x < display_xSize; ++x) {
      ppm.put((int)(IMAGE_BUFFER(x, y, 0) * 255.0));  // red
      ppm.put((int)(IMAGE_BUFFER(x, y, 1) * 255.0));  // green
      ppm.put((int)(IMAGE_BUFFER(x, y, 2) * 255.0));  // blue
    }
  }

  ppm.close();

  return RD_OK;
}

int pnm_write_pixel(int x, int y, const float rgb[]) {
  if (x < 0 || y < 0 || x >= display_xSize || y >= display_ySize) {
    return RD_OK;
  }
  IMAGE_BUFFER(x, y, 0) = std::clamp(rgb[0], ZERO, ONE);
  IMAGE_BUFFER(x, y, 1) = std::clamp(rgb[1], ZERO, ONE);
  IMAGE_BUFFER(x, y, 2) = std::clamp(rgb[2], ZERO, ONE);
  return RD_OK;
}

int pnm_read_pixel(int x, int y, float rgb[]) {
  rgb[0] = std::clamp(IMAGE_BUFFER(x, y, 0), ZERO, ONE);
  rgb[1] = std::clamp(IMAGE_BUFFER(x, y, 1), ZERO, ONE);
  rgb[2] = std::clamp(IMAGE_BUFFER(x, y, 2), ZERO, ONE);
  return RD_OK;
}

int pnm_set_background(const float rgb[]) {
  pnm_bg_color[0] = std::clamp(rgb[0], ZERO, ONE);
  pnm_bg_color[1] = std::clamp(rgb[1], ZERO, ONE);
  pnm_bg_color[2] = std::clamp(rgb[2], ZERO, ONE);
  return RD_OK;
}

int pnm_clear(void) {
  for (int x = 0; x < display_xSize; ++x) {
    for (int y = 0; y < display_ySize; ++y) {
      IMAGE_BUFFER(x, y, 0) = pnm_bg_color[0];
      IMAGE_BUFFER(x, y, 1) = pnm_bg_color[1];
      IMAGE_BUFFER(x, y, 2) = pnm_bg_color[2];
    }
  }
  return RD_OK;
}
