#ifndef PNM_DISPLAY_H
#define PNM_DISPLAY_H

#if defined(__cplusplus)
extern "C" {
#endif

// Allocate image memory
int pnm_init_display(void);

// Free image memory
int pnm_end_display(void);

// Clear the image array and store frame number
int pnm_init_frame(int);

// Write frame to file
int pnm_end_frame(void);

// Set the value at (x, y) to the rgb color
int pnm_write_pixel(int x, int y, const float rgb[]);

// Return the color value at (x, y) to rgb
int pnm_read_pixel(int x, int y, float rgb[]);

// Set the value of the background color (doesn't change the image)
int pnm_set_background(const float rgb[]);

// Set the entire image to the background color
int pnm_clear(void);

#if defined(__cplusplus)
}
#endif

#endif /* PNM_DISPLAY_H */
