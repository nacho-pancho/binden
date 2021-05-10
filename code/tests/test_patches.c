#include <stdio.h>
#include <stdlib.h>

#include "pnm.h"
#include "image.h"
#include "templates.h"
#include "patches.h"

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
    //
    //
    //
    const int m = img->info.height;
    const int n = img->info.width;
    //
    // create template
    //
    const int radius = 3;
    const int norm = 2;
    const int exclude_center = 0;
    patch_template_t* tpl;
    patch_t* pat;

    tpl = generate_ball_template ( radius, norm, exclude_center );
    pat = alloc_patch ( tpl->k );
    //
    // coordinate access
    //
    for ( int i = 0 ; i < m ; ++i ) {
        for ( int j = 0 ; j < n ; ++j ) {
            get_patch ( img, tpl, i, j, pat );
            int s = 0;
            for ( int r = 0 ; r < tpl->k ; ++r ) {
                s += pat->values[ r ];
            }
            set_pixel ( img, i, j, s / tpl->k );
        }
    }
    //
    // linear access
    //
    linear_template_t* ltpl = linearize_template ( tpl, m, n );
    for ( int i = 0, li = 0 ; i < m ; ++i ) {
        for ( int j = 0 ; j < n ; ++j, ++li ) {
            get_linear_patch ( img, ltpl, i, j, pat );
            int s = 0;
            for ( int r = 0 ; r < tpl->k ; ++r ) {
                s += pat->values[ r ];
            }
            set_linear_pixel ( img, li, s / tpl->k );
        }
    }
    //
    //
    //
    snprintf ( ofname, 128, "blurred_%s", fname );
    int res = write_pnm ( ofname, img );
    if ( res != RESULT_OK ) {
        fprintf ( stderr, "error writing image %s.\n", ofname );
    }
    print_patch ( pat );
    print_patch_fancy ( pat, tpl );
    print_binary_patch_fancy ( pat, tpl );
    free_patch ( pat );
    free_linear_template ( ltpl );
    free_patch_template ( tpl );
    pixels_free ( img->pixels );
    free ( img );
    return res;
}
