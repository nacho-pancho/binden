#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "pnm.h"
#include "image.h"
#include "templates.h"
#include "patches.h"
#include "config.h"

float * create_gaussian_weights ( patch_template_t* tpl, const float sigma ) {
    const int k = tpl->k;
    float* weights = ( float* ) malloc ( k * sizeof( float ) );
    float n = 0.0f;
    for ( int r = 0 ; r < k ; ++r ) {
        const float i = fabs ( ( double ) tpl->coords[ r ].i );
        const float j = fabs ( ( double ) tpl->coords[ r ].j );
        const float w = exp ( -0.5 * ( i * i + j * j ) / ( sigma * sigma ) );
        weights[ r ] = w;
        n += w;
    }
    // normalize  so that sum is 1
    for ( int r = 0 ; r < k ; ++r ) {
        weights[ r ] /= n;
    }
    return weights;
}

double patch_dist ( const patch_t* a, const patch_t* b, const float* weights ) {
    const int k = a->k;
    double dist = 0;
    const pixel_t* pa = a->values;
    const pixel_t* pb = b->values;
    for ( int r = 0 ; r < k ; ++r ) {
        dist += weights[ r ] * fabs ( ( double ) ( pa[ r ] - pb[ r ] ) );
    }
    return dist;
}

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
    // non-local means
    // search a window of size R
    //
    const float sigma = cfg.nlm_window_scale;
    const int R = cfg.search_radius;
    const double h = cfg.nlm_weight_scale;
    const double C = -0.5 / ( h * h );
    float* weights = create_gaussian_weights ( tpl, sigma );
    printf("NLM; R=%d h=%f C=%f\n",R, h,C);

    linear_template_t* ltpl = linearize_template ( tpl, m, n );
    for ( int i = 0, li = 0 ; i < m ; ++i ) {
        for ( int j = 0 ; j < n ; ++j, ++li ) {
            double y = 0;
            double norm = 0;
            int count = 0;
            get_linear_patch ( img, ltpl, i, j, pat );
            int di0 = i > R     ? i - R : 0;
            int di1 = i < ( m - R ) ? i + R : m;
            int dj0 = j > R     ? j - R : 0;
            int dj1 = j < ( n - R ) ? j + R : n;
            //printf("di0 %d di1 %d dj0 %d dj1 %d\n",di0,di1,dj0,dj1);
            for ( int di = di0 ; di < di1 ; ++di ) {
                for ( int dj = dj0 ; dj < dj1 ; ++dj ) {
                    get_linear_patch ( img, ltpl, di, dj, pot );
                    const double d = patch_dist ( pat, pot, weights );
                    const double w = exp ( C * d );
                    y += w * get_pixel ( img, di, dj );
                    norm += w;
                    count++;
                }
            }
            const int x = ( int ) ( 0.5 + y / norm );
            set_linear_pixel ( &out, li, x );
            //set_linear_pixel(&out,li,count/100);
        }
        printf("line %06d \n",i);
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
    free ( weights );
    return res;
}
