#ifndef IMAGE_H
#define IMAGE_H

#include "types.h"
#include "pnm.h"

int get_pixel(const image_t* pimg, const int i, const int j);

int get_linear_pixel(const image_t* pimg, const int li);

int set_pixel(image_t* pimg, const int i, const int j, const pixel_t val);

void set_linear_pixel(image_t* pimg, const int li, const pixel_t val);

#endif