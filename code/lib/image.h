#ifndef IMAGE_H
#define IMAGE_H

#include "types.h"
#include "pnm.h"

int get_pixel(const image_t* pimg, int i, int j);

int get_linear_pixel(const image_t* pimg, int li);

#endif