#include "image.h"

/*---------------------------------------------------------------------------------------*/

int get_pixel(const image_t* pimg, int i, int j) {
    const image_info_t info = pimg->info;
    return (i < 0) || (i >= info.height) || (j < 0) || (j >= info.width) ? 0 : pimg->pixels[i*info.width+j];
}

/*---------------------------------------------------------------------------------------*/

int get_linear_pixel(const image_t* pimg, int li) {
    const int npixels = pimg->info.width * pimg->info.height;
    return (li < 0) || (li >= npixels) ? 0 : pimg->pixels[li];
}
