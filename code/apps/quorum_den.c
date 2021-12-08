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
#include "bitfun.h"
#include "config.h"
#include "logging.h"

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

    const double p0 = cfg->p01;
    const double p1 = cfg->p10;
    const double pe = p0 + p1;
    const double pc = 1.0 - pe;
    /*
    * we define the thresholds:
    *  t0 = 2p1(1-p0)/(1+p1-p0)
    *  t1 = 2p0(1-p1)/(1+p0-p1)
    * so that
    * 
    * if z = 0:
    * x = 0 if n_0/n >= t0
    * x = 1 otherwise
    * 
    * if z = 1:
    * x = 1 if n_1/n >= t1
    * x = 0 otherwise
    * 
    */  
    const double t0 = 2.0*p1*(1.0-p0) / ( 1.0+p1-p0);
    const double t1 = 2.0*p0*(1.0-p1) / ( 1.0+p0-p1);
    printf("DUDE: p0=%f p1=%f t0=%f t1=%f\n",p0,p1,t0,t1);

    const int m = in->info.height;
    const int n = in->info.width;
    const index_t total = m * n;
    char* lookup_table = ( char* ) calloc ( 2*(k + 1),  sizeof( char ) );
    info( "Lookup table:\n");
    for ( int r = 0 ; r <= k  ; ++r ) {
        const double n  = ( double ) quorum_freq[ r ]  / ( double ) total;
        const double q1 = ( ( double ) quorum_freq_1[ r ] ) / ( ( double ) quorum_freq[ r ] );        
        const double q0 = 1.0 - q1;
        const char x0 = q0 >= t0 ? 0 : 1;
        const char x1 = q1 >= t1 ? 1 : 0;
        lookup_table[2*r]  = x0;
        lookup_table[2*r+1] = x1;
    	info ( "S=%3d P(S)=%8.6f P(0|S) %8.6f t0 %8.6f x(0,S) %d P(1|S) %8.6f t1 %8.6f x(1,S) %d\n", r, n, q0, t0, x0, q1, t1, x1 );
    }

    index_t changed = 0;
    for ( int i = 0, li = 0 ; i < m ; ++i ) {
        for ( int j = 0 ; j < n ; ++j, ++li ) {
            //
            // denoising rule:
            //
            const int z = get_linear_pixel ( in, li );
            const int S = quorum_map[ li ];
            const int x = lookup_table[(S<<1)+z];
            if (x!=z) {
                set_linear_pixel ( out, li, x );
                changed ++;
            }
        }
    }
    info ( "Changed %ld pixels\n", changed );
    free ( lookup_table );
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
