/**
 * full binary version of non-local means
 * input and output images are binary
 * patches are binary
 * distances are binary, with some weights
 * the patches are stored in a tree structure
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
#include "stats.h"
#include "config.h"
#include "logging.h"

/*---------------------------------------------------------------------------------------*/

index_t apply_denoiser ( image_t* out, const image_t* img,
                         const patch_template_t* tpl, patch_node_t* stats, config_t* cfg ) {

    const double p01 = cfg->p01;
    const double p10 = cfg->p10;
    const double pe = p01 + p10;
    const double pc = 1.0 - pe;

    const index_t maxd = (int)((double)tpl->k * pe *2.0 + 0.5);

    double w[ maxd+1 ];
    for ( index_t d = 0 ; d <= maxd ; ++d ) {
        w[ d ] = 1.0 / ( d + 1.0 );
    }
    patch_t* Pij = alloc_patch ( tpl->k );
    index_t changed = 0;
    const int m = img->info.height;
    const int n = img->info.width;
    index_t no_neigh = 0;
    for ( int i = 0, li = 0 ; i < m ; ++i ) {
        for ( int j = 0 ; j < n ; ++j, ++li ) {
            get_patch ( img, tpl, i, j, Pij );
            double y = 0;
            double norm = 0;
            neighbor_list_t neighbors = find_neighbors ( stats, Pij, maxd );
            if (neighbors.number == 0) {
                no_neigh ++;
            }
            for ( int i = 0 ; i < neighbors.number ; ++i ) {
                const index_t d = neighbors.neighbors[ i ].dist;
                const patch_node_t* node = neighbors.neighbors[ i ].patch_node;
                y += w[ d ] * (double) node->counts;
                norm += w[ d ] * (double) node->occu;
            }
            free ( neighbors.neighbors );
            const pixel_t z = get_linear_pixel ( img, li );
            const pixel_t x = (pixel_t) cfg->denoiser ( z, y, norm, p01, p10 );
            if ( z != x ) {
                set_linear_pixel ( out, li, x );
                changed++;
            }
        }
        if ( ( i > 0 ) &&!( i % 100 ) ) {
            info ( "row %d changed %ld ( %7.4f%% )\n", i, changed, ( double ) changed * 100.0 / ( double ) li );
        }
    }
    info("no neighbors found in %lu cases.\n",no_neigh);    
    free_patch ( Pij );
    return changed;
}

/*---------------------------------------------------------------------------------------*/

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

    image_t out;
    out.info = img->info;
    out.pixels = pixels_copy ( &img->info, img->pixels );

    patch_template_t* tpl;

    if ( ( !cfg.template_file ) || ( !strlen ( cfg.template_file ) ) ) {
        tpl = generate_ball_template ( cfg.template_radius, cfg.template_norm, cfg.template_center ? 0 : 1 );
        sort_template ( tpl, 1 );
    } else {
        tpl = read_template ( cfg.template_file );
        if ( !tpl->k ) {
            fprintf ( stderr, "could not load template from %s.\n", cfg.template_file );
            pixels_free ( img->pixels );
            free ( img );
            exit ( RESULT_ERROR );
        }
    }
    info("Using the following template:\n");
    sort_template(tpl,1);
    //print_template(tpl);
    //
    // gather patch stats
    //
    patch_node_t* stats;
    if ( cfg.stats_file ) {
        //
        // load patches from a file
        // the patches from this file need not have been
        // generated from this image, but the template
        // must have been the same
        //
        info ( "loading patch statistics from file....\n" );
        stats = load_stats ( cfg.stats_file );
        if ( !stats ) {
            fprintf ( stderr, "could not load stats from %s.\n", cfg.stats_file );
            free_patch_template ( tpl );
            pixels_free ( img->pixels );
            free ( img );
            return RESULT_ERROR;
        }
    } else {
        info ( "gathering patch stats from image....\n" );
        stats = gather_patch_stats ( img, pre, tpl, NULL, NULL );
    }
#if 1
    const index_t minoccu = 100;
    info ( "clustering patches....\n" );
    patch_node_t * clustered = cluster_stats ( stats, tpl->k, cfg.max_dist, minoccu, cfg.max_clusters );
    //print_patch_stats ( clustered, tpl->k );
#else
    patch_node_t* clustered = stats;
#endif
    info ( "denoising....\n" );
    apply_denoiser ( &out, img, tpl, clustered, &cfg );

    info ( "saving result...\n" );
    int res = write_pnm ( cfg.output_file, &out );
    if ( res != RESULT_OK ) {
        fprintf ( stderr, "error writing image %s.\n", cfg.output_file );
    }

    info ( "finishing...\n" );
    if (clustered != stats)
        free_node ( clustered );
    free_node ( stats );
    free_patch_template ( tpl );
    pixels_free ( img->pixels );
    pixels_free ( out.pixels );
    if ( pre != img ) {
        pixels_free ( pre->pixels );
        free ( pre );
    }
    free ( img );
    return res;
}

/*---------------------------------------------------------------------------------------*/
