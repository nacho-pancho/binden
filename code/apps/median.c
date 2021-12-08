#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "pnm.h"
#include "image.h"
#include "templates.h"
#include "patches.h"
#include "config.h"
#include "logging.h"

int main ( int argc, char* argv[] ) {

    config_t cfg = parse_opt ( argc, argv );

    image_t* img = read_pnm ( cfg.input_file );
    if ( img == NULL ) {
        fprintf ( stderr, "error opening image %s.\n", cfg.input_file );
        return RESULT_ERROR;
    }
    if ( img->info.result != RESULT_OK ) {
        fprintf ( stderr, "error reading image %s.\n", cfg.input_file );
        pixels_free ( img->pixels );
        free ( img );
        return RESULT_ERROR;
    }
    if ( img->info.maxval > 1 ) {
        fprintf ( stderr, "only binary images supported.\n" );
        pixels_free ( img->pixels );
        free ( img );
        return RESULT_ERROR;
    }
    patch_template_t* tpl;
    if ( !cfg.template_file || !strlen(cfg.template_file)) {
        fprintf ( stderr, "a template is required for this method.\n" );
        pixels_free ( img->pixels );
        free ( img );
        return RESULT_ERROR;
    }
    tpl = read_template ( cfg.template_file );
    if (!tpl) {
        fprintf ( stderr, "missing or invalid template file %s.\n",cfg.template_file );
        pixels_free ( img->pixels );
        free ( img );
        return RESULT_ERROR;
    }
    image_t out;
    out.info = img->info;
    out.pixels = pixels_copy ( &img->info, img->pixels );
    //
    //
    //
    const int m = img->info.height;
    const int n = img->info.width;
    //
    // create template
    //
    patch_t* pat, * pot;


    pat = alloc_patch ( tpl->k );
    pot = alloc_patch ( tpl->k );
    //
    // median of neighborhood
    //
    const int k = tpl->k;
    linear_template_t* ltpl = linearize_template ( tpl, m, n );
    for ( int i = 0, li = 0 ; i < m ; ++i ) {
        for ( int j = 0 ; j < n ; ++j, ++li ) {
            int count = 0;
            get_linear_patch ( img, ltpl, i, j, pat );
            long a = 0;
            for (int r = 0; r < k; ++r) {
                a += pat->values[r];
            } 
            const int x = ((a<<1) >= k) ? 1 : 0;
            set_linear_pixel ( &out, li, x );
        }
    }
    //
    //
    //
    int res = write_pnm ( cfg.output_file, &out );
    if ( res != RESULT_OK ) {
        fprintf ( stderr, "error writing image %s.\n", cfg.output_file );
    }
    free_patch ( pot );
    free_patch ( pat );
    free_linear_template ( ltpl );
    free_patch_template ( tpl );
    pixels_free ( img->pixels );
    pixels_free ( out.pixels );
    free ( img );
    return res;
}
