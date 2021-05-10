#include <stdio.h>
#include <stdlib.h>

#include "pnm.h"
#include "image.h"

int main ( int argc, char* argv[] ) {
    char ofname[ 128 ];
    if ( argc < 2 ) {
        fprintf ( stderr, "usage: %s <image>.\n", argv[ 0 ] );
        return RESULT_ERROR;
    }
    const char* fname = argv[ 1 ];
    image_t* img = read_pnm ( fname );
    if ( img == NULL ) {
        fprintf ( stderr, "error opening image %s.\n", fname );
        return RESULT_ERROR;
    }
    if ( img->info.result != RESULT_OK ) {
        fprintf ( stderr, "error reading image %s.\n", fname );
        pixels_free ( img->pixels );
        free ( img );
        return RESULT_ERROR;
    }
    snprintf ( ofname, 128, "mirror_%s", fname );
    /**
     * check mirror  using linear coordinates
     */
    const int n = img->info.width * img->info.height;
    for ( int i = 0 ; i < n / 2 ; ++i ) {
        set_linear_pixel ( img, n - 1 - i, get_linear_pixel ( img, i ) );
    }
    int res = write_pnm ( ofname, img );
    if ( res != RESULT_OK ) {
        fprintf ( stderr, "error writing image %s.\n", ofname );
    }
    /**
     * invert image using 2D coordinates
     */
    for ( int i = 0 ; i < img->info.height ; ++i ) {
        for ( int j = 0 ; j < img->info.width ; ++j ) {
            set_pixel ( img, i, j, img->info.maxval - get_pixel ( img, i, j ) );
        }
    }
    snprintf ( ofname, 128, "inverted_mirror_%s", fname );
    res = write_pnm ( ofname, img );
    if ( res != RESULT_OK ) {
        fprintf ( stderr, "error writing image %s.\n", ofname );
    }

    pixels_free ( img->pixels );
    free ( img );
    return res;
}
