#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

#include "types.h"

typedef int16_t pixel_t;

typedef struct image_info {
    int width;  // width of image
    int height; // height of image
    int channels; // number of channels
    int type;   // reserved for PNM: PNM image type (grayscale, color) using PNM conventions
    int depth;  // number of *bits* per pixel, usually 8 or 16
    int maxval; // maximum value, usually 255 or 65535
    int result;  // result of reading: image is valid only if this is RESULT_OK
    int encoding; // reserved for PNM I/O: PNM_ASCII or PNM_BINARY
} image_info_t;

typedef struct image {
    image_info_t info;
    //
    // pixel data, stored in row major order (first row pixels, then second row, etc.)
    // multichannel pixels are stored in interleaved fashion: r,g,b,r,g,b,r,g,b...
    //
    pixel_t * pixels;
} image_t;

//
//---------------------------------------------------------------------------------------------
//
pixel_t * pixels_alloc ( const image_info_t * info );
//
//---------------------------------------------------------------------------------------------
//
void pixels_free ( pixel_t * pix );



pixel_t * pixels_copy ( const image_info_t* info, const pixel_t * pix );

image_t* image_copy ( const image_t * src );

void pixels_copyto ( image_t* dest, const image_t * src );

int get_pixel ( const image_t * pimg, const int i, const int j );

int get_linear_pixel ( const image_t * pimg, const int li );

void set_pixel ( image_t * pimg, const int i, const int j, const pixel_t val );

void set_linear_pixel ( image_t * pimg, const int li, const pixel_t val );

#endif
