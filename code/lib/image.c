#include <assert.h>
#include "image.h"

/*---------------------------------------------------------------------------------------*/

int get_pixel(const image_t* pimg, const int i, const int j) {
    const image_info_t info = pimg->info;
    return (i < 0) || (i >= info.height) || (j < 0) || (j >= info.width) ? 0 : pimg->pixels[i*info.width+j];
}

/*---------------------------------------------------------------------------------------*/

int get_linear_pixel(const image_t* pimg, const int li) {
    const int npixels = pimg->info.width * pimg->info.height;
    return (li < 0) || (li >= npixels) ? 0 : pimg->pixels[li];
}

/*---------------------------------------------------------------------------------------*/

void set_pixel(image_t* pimg, const int i, const int j, const pixel_t val) {
    const image_info_t info = pimg->info;
    assert((i >= 0) && (i < info.height) && (j >= 0) && (j < info.width));
    pimg->pixels[i*info.width+j] = val;
}

/*---------------------------------------------------------------------------------------*/

void set_linear_pixel(image_t* pimg, const int li, const pixel_t val) {
    const int npixels = pimg->info.width * pimg->info.height;
    assert((li >= 0) && (li < npixels));
    pimg->pixels[li] = val;
}
