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
#include "patch_mapper.h"
#include "bitfun.h"
#include "stats.h"


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
    if ( img->info.maxval > 1) {
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
    const int m = img->info.height;
    const int n = img->info.width;
    //
    // create template
    //
    const int radius = 3;
    const int norm = 2;
    const int exclude_center = 1;
    const index_t maxd = 4;
    const double perr = 0.05;
    index_t w[maxd];
    for (index_t d = 0; d < maxd; ++d) {
        w[d] = 1024/(d+1);
    }

    patch_template_t* tpl;

    tpl = generate_ball_template ( radius, norm, exclude_center );
    //
    // non-local means
    // search a window of size R
    //
    printf ( "extracting patches....\n" );
    patch_node_t* stats = gather_patch_stats(img,img,tpl,NULL,NULL);

    printf ( "denoising....\n" );

    patch_t* Pij = alloc_patch(tpl->k);
    index_t changed = 0;
    for ( int i = 0, li = 0 ; i < m ; ++i ) {
        for ( int j = 0 ; j < n ; ++j, ++li ) {
            get_patch(img,tpl,i,j,Pij);
            index_t y = 0;
            index_t norm = 0;
            neighbor_list_t neighbors = find_neighbors ( stats, Pij, maxd);
            for (int i = 0; i < neighbors.number; ++i) {
                const index_t d = neighbors.neighbors[i].dist;
                if (d == 0) continue;
                const patch_node_t* node = neighbors.neighbors[i].patch_node;
                y += w[d-1]*node->counts;
                norm += w[d-1]*node->occu;
            }
            free(neighbors.neighbors);
	    // minimum Bayes risk rule
	    const double q = (0.5 + y) / (1.0 + norm);
            const pixel_t z = get_linear_pixel(img, li);
            if (z && (q < perr)) {
                set_linear_pixel ( &out, li, 0);
                changed++;
            } else if ((!z) && (q > (1.0-perr))) {
                set_linear_pixel ( &out, li, 1);
                changed++;
            }
        }
        if ((i > 0) &&!(i % 100)) {
            printf("row %d changed %ld\n",i,changed);
        }
    }

    printf ( "saving result...\n" );
    snprintf ( ofname, 128, "nlm_%s", fname );
    // OVERRIDE since writing raw binary type is broken
    out.info.type = 1;
    out.info.encoding = PNM_ASCII;
    int res = write_pnm ( ofname, &out );
    if ( res != RESULT_OK ) {
        fprintf ( stderr, "error writing image %s.\n", ofname );
    }


    printf ( "finishing...\n" );
    free_node(stats);
    free_patch_template ( tpl );
    pixels_free ( img->pixels );
    pixels_free ( out.pixels );
    free ( img );
    return res;
}
