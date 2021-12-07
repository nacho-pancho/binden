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
//#include "patch_mapper.h"
#include "bitfun.h"
#include "config.h"

/**
 * pseudo-image where each pixel's value contains the sum
 * of its patch samples
 * size (m x n)
 */
static index_t* quorum_map;

/**
 * frequency of each quorum value (size k + 1)
 * P(Q=q)
 */
static index_t* quorum_freq;

/**
 * frequency of each quorum value given that the center is 1
 * each quorum value. (size k + 1)
 */
static index_t* quorum_freq_1;

/**
 * frequency of each quorum value given that the center is 1
 * each quorum value. (size k + 1)
 */
static double* quorum_prob;

static index_t patch_sums ( const image_t* img, const image_t* ctximg, const patch_template_t* tpl,
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

static index_t apply_denoiser (
    image_t* out, const image_t* in,
    const index_t k,
    const index_t* quorum_map,
    const index_t* quorum_freq,
    const index_t* quorum_freq_1,
    const config_t* cfg ) {

    const double p01 = cfg->p01;
    const double p10 = cfg->p10;
    const double pe = p01 + p10;
    const double pc = 1.0 - pe;

    const int m = in->info.height;
    const int n = in->info.width;
    const index_t total = m * n;
    double* quorum_prob = ( double* ) calloc ( k + 1,  sizeof( double ) );

    for ( int r = 0 ; r < k ; ++r ) {
        quorum_prob[ r ] = ( 0.5 + ( double ) quorum_freq_1[ r ] ) / ( 1.0 + ( double ) quorum_freq[ r ] );
    }
    //info ( "estimating probabilities (Krichevskii-Trofimoff)....\n" );
    for ( int r = 0 ; r < k + 1 ; ++r ) {
        quorum_prob[ r ]   = ( 0.5 + ( double ) quorum_freq_1[ r ] ) / ( 1.0 + ( double ) quorum_freq[ r ] );
        const double PS  = ( double ) quorum_freq[ r ]  / ( double ) total;
        const double PS1 = ( double ) quorum_freq_1[ r ] / ( double ) total;
	//info ( "sum %3d P(S)=%8.6f P(1,S)=%8.6f P(1|S) %8.6f\n", r, PS, PS1, quorum_prob[ r ] );
    }

    info ( "denoising....\n" );
    index_t changed = 0;
    for ( int i = 0, li = 0 ; i < m ; ++i ) {
        for ( int j = 0 ; j < n ; ++j, ++li ) {
            //
            // denoising rule:
            //
            const pixel_t z = get_linear_pixel ( in, li );
            const int S = quorum_map[ li ];
            const double q = quorum_prob[ S ];
            // TODO: fix for non-symmetric!!
            if ( z && ( q < pe ) ) {
                //info("%d %d S=%d q=%8.6f: 1 -> 0\n",i,j,S,q);
                changed++;
                set_linear_pixel ( out, li, 0 );
            } else if ( ( !z ) && ( q > pc ) ) {
                set_linear_pixel ( out, li, 1 );
                //info("%d %d S=%d q=%6.6f: 0 -> 1\n",i,j,S,q);
                changed++;
            }
        }
    }
    info ( "Changed %ld pixels\n", changed );
    free ( quorum_prob );
    return changed;
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
    //
    // create template
    //
    //
    // non-local means
    // search a window of size R
    //
    info ( "computing sums....\n" );
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
    apply_denoiser ( &out, img, tpl->k, quorum_map, quorum_freq, quorum_freq_1, &cfg );
    //
    // second pass: using denoised for contexts
    //
    patch_sums ( img, &out, tpl, quorum_map, quorum_freq, quorum_freq_1 );
    apply_denoiser ( &out, img, tpl->k, quorum_map, quorum_freq, quorum_freq_1, &cfg );

    info ( "saving result to %s ...\n",cfg.output_file );
    int res = write_pnm ( cfg.output_file, &out );
    if ( res != RESULT_OK ) {
        fprintf ( stderr, "error writing image %s.\n", cfg.output_file );
    }


    info ( "finishing...\n" );
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
