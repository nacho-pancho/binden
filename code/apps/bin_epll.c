/**
 * binarized non-local means
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

#include "nlm_options.h"

int main ( int argc, char* argv[] ) {
    nlm_config_t cfg = parse_opt(argc,argv);

    image_t* img = read_pnm ( cfg.input_file );
    if ( img == NULL ) {
        fprintf ( stderr, "error opening image %s.\n", cfg.input_file );
        exit(RESULT_ERROR);
    }
    if ( img->info.result != RESULT_OK ) {
        fprintf ( stderr, "error reading image %s.\n", cfg.input_file );
        pixels_free ( img->pixels );
        free ( img );
        exit(RESULT_ERROR);
    }
    if ( img->info.maxval > 1) {
        fprintf ( stderr, "only binary images supported.\n" );
        pixels_free ( img->pixels );
        free ( img );
        return RESULT_ERROR;
    }

    if ((!cfg.template_file) || (!strlen(cfg.template_file))) {
      fprintf(stderr,"must specify template file.\n");
      exit(RESULT_ERROR);
    }
    patch_template_t* template = read_template(cfg.template_file);
    if (!template->k) {
      fprintf ( stderr, "could not load template from %s.\n",cfg.template_file );
        pixels_free ( img->pixels );
        free ( img );
        exit(RESULT_ERROR);      
    }

    if ((!cfg.stats_file) || (!strlen(cfg.stats_file))) {
      fprintf(stderr,"must specify stats file.\n");
        exit(RESULT_ERROR);      
    }
    patch_node_t* model = load_stats(cfg.stats_file);
    if (!model) {
      fprintf ( stderr, "could not load model from %s.\n",cfg.stats_file );
      free_patch_template(template);
        pixels_free ( img->pixels );
        free ( img );
        return RESULT_ERROR;      
    }

    const double perr = cfg.perr;
    image_t out;
    out.info = img->info;
    out.pixels = pixels_copy ( &img->info, img->pixels );
    //
    //
    //
    const int m = img->info.height;
    const int n = img->info.width;

    const index_t maxd = 4;
    index_t w[maxd];
    for (index_t d = 0; d < maxd; ++d) {
        w[d] = 1024/(d+1);
    }


    //
    // non-local means
    // search a window of size R
    //
    printf ( "extracting patches....\n" );
    patch_node_t* stats = gather_patch_stats(img,img,template,NULL,NULL);

    printf ( "denoising....\n" );

    patch_t* Pij = alloc_patch(template->k);
    index_t changed = 0;
    for ( int i = 0, li = 0 ; i < m ; ++i ) {
        for ( int j = 0 ; j < n ; ++j, ++li ) {
            get_patch(img,template,i,j,Pij);
            double y = 0;
            double norm = 0;
            neighbor_list_t neighbors = find_neighbors ( model, Pij, maxd);
            for (int i = 0; i < neighbors.number; ++i) {
                const index_t d = neighbors.neighbors[i].dist;
                if (d == 0) continue;
                const patch_node_t* node = neighbors.neighbors[i].patch_node;
#if 1
                y += w[d-1]*node->counts;
                norm += w[d-1]*node->occu;
#else
		// does not take occurence of node into account
                y += ((double)w[d-1])*((double) node->counts) / ((double) node->occu);
                norm += w[d-1];
#endif
            }
            free(neighbors.neighbors);

            const pixel_t z = get_linear_pixel(img, li);
            const pixel_t x = cfg.denoiser(z,y,norm,perr);
            if (z != x) {
                set_linear_pixel ( &out, li, x);
                changed++;
            }
	}
        if ((i > 0) &&!(i % 100)) {
            printf("row %d changed %ld\n",i,changed);
        }
    }

    printf ( "saving result to %s...\n", cfg.output_file );
    // OVERRIDE since writing raw binary type is broken
    out.info.type = 1;
    out.info.encoding = PNM_ASCII;
    int res = write_pnm ( cfg.output_file, &out );
    if ( res != RESULT_OK ) {
        fprintf ( stderr, "error writing image %s.\n", cfg.output_file );
    }


    printf ( "finishing...\n" );
    free_node(stats);
    free_node(model);
    free_patch_template ( template );
    pixels_free ( img->pixels );
    pixels_free ( out.pixels );
    free ( img );
    return res;
}
