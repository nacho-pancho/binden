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
#include "config.h"
#include "templates.h"
#include "logging.h"


/**
 * @brief DUDE denoiser for binary asymmetric channel
 * 
 * given p0 = P(0->1) and p1 = P(1->0)
 *  the counts of 0s,n0,  1s, n1, and the total occurrences of the context, n
 * the rule is given by:
 * 
 * if z = 0:
 * x = 0 if n_0/n >= 2p1(1-p0)/(1+p1-p0)
 * x = 1 otherwise
 * 
 * if z = 1:
 * x = 1 if n_1/n >= 2p0(1-p1)/(1+p0-p1)
 * x = 0 otherwise
 * 
 * @param out denoised image
 * @param in noisy image
 * @param tpl template to define context
 * @param stats context statistics
 * @param cfg program configuration
 * @return index_t number of changed pixels
 */
static index_t apply_denoiser (
    image_t* out, 
    const image_t* in, 
    const image_t* pre,
    const patch_template_t* tpl,
    patch_node_t* stats,
    config_t* cfg) {

    const double p0 = cfg->p01;
    const double p1 = cfg->p10;
    const double pe = p0 + p1;
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
    //info("DUDE: p0=%f p1=%f t0=%f t1=%f\n",p0,p1,t0,t1);
    const int m = in->info.height;
    const int n = in->info.width;
    const index_t total = m * n;

    index_t zeroed = 0, oned = 0;
    const index_t k = tpl->k;
    patch_t* Pij = alloc_patch ( k );
    for ( int i = 0, li = 0 ; i < m ; ++i ) {
        for ( int j = 0 ; j < n ; ++j, ++li ) {
            //
            // denoising rule:
            //
            get_patch ( pre, tpl, i, j, Pij );
            const pixel_t z = get_linear_pixel ( in, li );
            const patch_node_t* patch_stats  = get_patch_node( stats, Pij );
            if ( !z ) { // z = 0
                //const double q0 = (0.5 + (double)(patch_stats->occu-patch_stats->counts)) / (1.0 + (double) patch_stats->occu); 
                const double n0 = (double)(patch_stats->occu-patch_stats->counts);
                if (n0 < (t0 * (double)patch_stats->occu)) {
                    oned++;
                    set_linear_pixel ( out, li, 1 );
                }
            } else { // z = 1
                //const double q1 = (0.5 + (double) patch_stats->counts) / (1.0 + (double) patch_stats->occu); 
                const double n1 = (double)patch_stats->counts;
                if (n1 < (t1 * (double)patch_stats->occu)) {
                    set_linear_pixel ( out, li, 0 );
                    //info("%d %d S=%d q=%6.6f: 0 -> 1\n",i,j,S,q);
                    zeroed++;
                }
            }
        }
    }
    free_patch ( Pij );
    info ( "changed : 0->1 (%8.4f%%) 1->0 (%8.4f%%) total (%8.4f%%) pixels\n", 
        100.0*((double)oned)/((double)total), 
        100.0*((double)zeroed)/((double)total),
        100.0*((double)(zeroed+oned))/((double)total));
    info ( "expected: 0->1 (%8.4f%%) 1->0 (%8.4f%%) total (%8.4f%%) pixels\n", 
        100.0*p0, 100.0*p1, 100.0*pe);
    return (oned+zeroed);
}



int main ( int argc, char* argv[] ) {

    image_t* out = NULL;
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
    printf("initial output image\n");
    out = image_copy(img);

    image_t* pre = NULL;
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
    } else {
        printf("initial prefiltered image\n");
        pre = image_copy ( img );
    }
    //
    //
    //
    //
    // create template
    //
    sort_template(tpl,1); 
    //
    // non-local means
    // search a window of size R
    //
    patch_node_t* stats = NULL;
    if ( cfg.stats_file ) {
        //
        // if statistics are precomputed
        // the algorithm is run only once using the stats from the file
        //
        stats = load_stats ( cfg.stats_file );
        if ( !stats ) {
            fprintf ( stderr, "could not load stats from %s.\n", cfg.stats_file );
            free_patch_template ( tpl );
            pixels_free ( img->pixels );
            free ( img );
            return RESULT_ERROR;
        }
        apply_denoiser ( out, img, img, tpl, stats, &cfg);
    } else {
        //
        // if stats are computed on this image, we have the option of re-running the algorithm
        // multiple times, using the denoised output for gathering stats
        //
        for (int i = 0; i < cfg.iterations; i++) {
            info ("iteration %d\n",i);
            stats = gather_patch_stats ( img, pre, tpl, NULL, NULL );        
            apply_denoiser ( out, img, pre, tpl, stats, &cfg);
            free_node(stats);
            stats = 0;
            // prefiltered for next iter is output from this iter
            pixels_copyto(pre, out);
        }
    }

    int res = write_pnm ( cfg.output_file, out );
    if ( res != RESULT_OK ) {
        fprintf ( stderr, "error writing image %s.\n", cfg.output_file );
    }
    free_node(stats);
    free_patch_template ( tpl );
    pixels_free ( img->pixels );
    pixels_free ( out->pixels );
    pixels_free ( pre->pixels );
    free ( img );
    free ( out );
    free ( pre );
    return res;
}
