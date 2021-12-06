/**
 * Discrete Universal DEnoiser
 *
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
#include "stats.h"
#include "bitfun.h"
#include "nlm_options.h"
#include "templates.h"
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


index_t apply_denoiser (
    image_t* out, const image_t* in,
    const double perr,
    const patch_template_t* tpl,
    patch_node_t* stats) {

    const double p0 = perr;
    const double p1 = 1.0 - perr;

    const int m = in->info.height;
    const int n = in->info.width;
    const index_t total = m * n;

    printf ( "denoising....\n" );
    index_t changed = 0;
    const index_t k = tpl->k;
    patch_t* Pij = alloc_patch ( k );
    for ( int i = 0, li = 0 ; i < m ; ++i ) {
        for ( int j = 0 ; j < n ; ++j, ++li ) {
            //
            // denoising rule:
            //
            get_patch ( in, tpl, i, j, Pij );
            const pixel_t z = get_linear_pixel ( in, li );
            const patch_node_t* patch_stats  = get_patch_node( stats, Pij );
            const double q = (0.5 + (double)patch_stats->counts) / (1.0 + (double) patch_stats->occu); 
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
    free_patch ( Pij );
    printf ( "Changed %ld pixels\n", changed );
    free ( quorum_prob );
    return changed;
}

int main ( int argc, char* argv[] ) {

    char ofname[ 128 ];
    image_t out;
    nlm_config_t cfg = parse_opt ( argc, argv );

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

    image_t* pre = img;
    if ( cfg.prefiltered_file != NULL ) {
        pre = read_pnm ( cfg.prefiltered_file );
        if ( pre->info.result != RESULT_OK ) {
            fprintf ( stderr, "error reading prefiltered image %s.\n", cfg.prefiltered_file );
            pixels_free ( pre->pixels );
            pixels_free ( img->pixels );
            free ( img );
            free ( pre );
            return RESULT_ERROR;
        }
    }

    out.info = img->info;
    out.pixels = pixels_copy ( &img->info, img->pixels );
    //
    //
    //
    //
    // create template
    //
    const int radius = 2;
    const int norm = 2;
    const int exclude_center = 1;
    const double perr = 0.05;

    patch_template_t* tpl;

    tpl = read_template( cfg.template_file ); //generate_ball_template ( radius, norm, exclude_center );
    //
    // non-local means
    // search a window of size R
    //
    printf ( "computing sums....\n" );
    const index_t n = img->info.width;
    const index_t m = img->info.height;
    const index_t npatches = m * n;
    //quorum_map    = ( index_t* ) calloc ( npatches, sizeof( index_t ) );
    //quorum_freq   = ( index_t* ) calloc ( tpl->k + 1,  sizeof( index_t ) );
    //quorum_freq_1 = ( index_t* ) calloc ( tpl->k + 1,  sizeof( index_t ) );
    //
    // first pass:  gather patch stats
    //
    patch_node_t* stats;
    if ( cfg.stats_file ) {
        //
        // load patches stats from a file
        //
        printf ( "loading patch statistics from file....\n" );
        stats = load_stats ( cfg.stats_file );
        if ( !stats ) {
            fprintf ( stderr, "could not load stats from %s.\n", cfg.stats_file );
            free_patch_template ( tpl );
            pixels_free ( img->pixels );
            free ( img );
            return RESULT_ERROR;
        }
    } else {
        printf ( "gathering patch stats from image....\n" );
        stats = gather_patch_stats ( img, pre, tpl, NULL, NULL );
    }

    apply_denoiser ( &out, img, perr, tpl, stats);
    //
    // second pass: using denoised contexts
    //
    //stats = gather_patch_stats ( img, pre, tpl, NULL, NULL );
    //apply_denoiser ( &out, img, perr, tpl->k, quorum_map, quorum_freq, quorum_freq_1 );

    printf ( "saving result...\n" );
    int res = write_pnm ( cfg.output_file, &out );
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
