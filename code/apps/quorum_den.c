/**
 * quorum denoiser
 * given a template, patches are classified according to their
 * sum. Statistics are gathered for each sum value, and
 * then a Bayesian minimum-risk rule is used to decide upon
 * each center pixel.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h> // for memcmp

#include "pnm.h"
#include "image.h"
#include "templates.h"
#include "patches.h"
#include "patch_mapper.h"
#include "bitfun.h"

/**
 * pseudo-image where each pixel's value contains the sum
 * of its patch samples
 * size (m x n)
 */
index_t* quorum_map;

/**
 * frequency of each quorum value (size k + 1)
 * P(Q=q)
 */
index_t* quorum_freq;

/**
 * frequency of each quorum value given that the center is 1
 * each quorum value. (size k + 1)
 */
index_t* quorum_freq_1;

/**
 * frequency of each quorum value given that the center is 1
 * each quorum value. (size k + 1)
 */
double* quorum_prob;

index_t patch_sums ( const image_t* img, const image_t* ctximg, const patch_template_t* tpl,
                     index_t* quorum_map, index_t* quorum_freq, index_t* quorum_freq_1 ) {
    //
    // determine total number of patches in image
    //
    if ( ctximg == NULL ) {
        ctximg = img;
    }
    const index_t n = img->info.width;
    const index_t m = img->info.height;
    index_t total = 0;
    patch_t* p = alloc_patch ( tpl->k );
    linear_template_t* ltpl = linearize_template ( tpl, m, n );
    for ( int i = 0, li = 0 ; i < m ; ++i ) {
        for ( int j = 0 ; j < n ; ++j, ++li ) {
            get_linear_patch ( ctximg, ltpl, i, j, p );
            //
            // sum
            //
            index_t a = 0;
            for ( int k = 0 ; k < ltpl->k ; ++k ) {
                a += p->values[ k ];
            }
            quorum_map[ li ] = a;
            quorum_freq[ a ]++;
            if ( get_linear_pixel ( img, li ) ) {
                quorum_freq_1[ a ]++;
            }
            total++;
        }
    }
    free_linear_template ( ltpl );
    free_patch ( p );
    return total;
}

index_t apply_denoiser (
    image_t* out, const image_t* in,
    const double perr,
    const index_t k,
    const index_t* quorum_map,
    const index_t* quorum_freq,
    const index_t* quorum_freq_1 ) {

    const double p0 = perr;
    const double p1 = 1.0 - perr;

    const int m = in->info.height;
    const int n = in->info.width;
    const index_t total = m * n;
    double* quorum_prob = ( double* ) calloc ( k + 1,  sizeof( double ) );

    for ( int r = 0 ; r < k ; ++r ) {
        quorum_prob[ r ] = ( 0.5 + ( double ) quorum_freq_1[ r ] ) / ( 1.0 + ( double ) quorum_freq[ r ] );
    }
    printf ( "estimating probabilities (Krichevskii-Trofimoff)....\n" );
    for ( int r = 0 ; r < k + 1 ; ++r ) {
        quorum_prob[ r ]   = ( 0.5 + ( double ) quorum_freq_1[ r ] ) / ( 1.0 + ( double ) quorum_freq[ r ] );
        const double PS  = ( double ) quorum_freq[ r ]  / ( double ) total;
        const double PS1 = ( double ) quorum_freq_1[ r ] / ( double ) total;
        printf ( "sum %3d P(S)=%8.6f P(1,S)=%8.6f P(1|S) %8.6f\n", r, PS, PS1, quorum_prob[ r ] );
    }

    printf ( "denoising....\n" );
    index_t changed = 0;
    for ( int i = 0, li = 0 ; i < m ; ++i ) {
        for ( int j = 0 ; j < n ; ++j, ++li ) {
            //
            // denoising rule:
            //
            const pixel_t z = get_linear_pixel ( in, li );
            const int S = quorum_map[ li ];
            const double q = quorum_prob[ S ];
            if ( z && ( q < p0 ) ) {
                //printf("%d %d S=%d q=%8.6f: 1 -> 0\n",i,j,S,q);
                changed++;
                set_linear_pixel ( out, li, 0 );
            } else if ( ( !z ) && ( q > p1 ) ) {
                set_linear_pixel ( out, li, 1 );
                //printf("%d %d S=%d q=%6.6f: 0 -> 1\n",i,j,S,q);
                changed++;
            }
        }
    }
    printf ( "Changed %ld pixels\n", changed );
    free ( quorum_prob );
    return changed;
}

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
    if ( img->info.maxval > 1 ) {
        fprintf ( stderr, "only binary images supported.\n" );
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
    //
    // create template
    //
    const int radius = 8;
    const int norm = 2;
    const int exclude_center = 1;
    const double perr = 0.05;

    patch_template_t* tpl;

    tpl = generate_ball_template ( radius, norm, exclude_center );
    //
    // non-local means
    // search a window of size R
    //
    printf ( "computing sums....\n" );
    const index_t n = img->info.width;
    const index_t m = img->info.height;
    const index_t npatches = m * n;
    quorum_map    = ( index_t* ) calloc ( npatches, sizeof( index_t ) );
    quorum_freq   = ( index_t* ) calloc ( tpl->k + 1,  sizeof( index_t ) );
    quorum_freq_1 = ( index_t* ) calloc ( tpl->k + 1,  sizeof( index_t ) );
    //
    // first pass: noisy sums
    //
    patch_sums ( img, img, tpl, quorum_map, quorum_freq, quorum_freq_1 );
    apply_denoiser ( &out, img, perr, tpl->k, quorum_map, quorum_freq, quorum_freq_1 );
    //
    // second pass: using denoised for contexts
    //
    patch_sums ( img, &out, tpl, quorum_map, quorum_freq, quorum_freq_1 );
    apply_denoiser ( &out, img, perr, tpl->k, quorum_map, quorum_freq, quorum_freq_1 );

    printf ( "saving result...\n" );
    snprintf ( ofname, 128, "quo_%s", fname );
    int res = write_pnm ( ofname, &out );
    if ( res != RESULT_OK ) {
        fprintf ( stderr, "error writing image %s.\n", ofname );
    }


    printf ( "finishing...\n" );
    free ( quorum_prob );
    free ( quorum_freq_1 );
    free ( quorum_freq );
    free ( quorum_map );
    free_patch_template ( tpl );
    pixels_free ( img->pixels );
    pixels_free ( out.pixels );
    free ( img );
    return res;
}
