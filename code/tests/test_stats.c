#include <stdio.h>
#include <stdlib.h>

#include "pnm.h"
#include "image.h"
#include "templates.h"
#include "patches.h"
#include "stats.h"

int main ( int argc, char* argv[] ) {

    if ( argc < 3 ) {
        fprintf ( stderr, "usage: %s <image> <template>.\n", argv[ 0 ] );
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
    //
    //
    //
    //
    // create template
    //
    patch_template_t* tpl;
    
    tpl = read_template( argv[2] ); 
    //tpl = generate_ball_template ( radius, norm, exclude_center );
    //
    // coordinate access
    //
    patch_node_t* stats_tree;
    printf ( "image alphabet size %d\n", img->info.maxval + 1 );
    stats_tree = gather_patch_stats ( img, img, tpl, NULL, NULL );
    printf ( "number of pixels %d\n", img->info.width * img->info.height );
    printf("=================\n");
    print_stats_summary ( stats_tree, ">" );
    print_patch_stats( stats_tree, tpl->k );
    save_stats ( "test.stats", stats_tree );
    patch_node_t* loaded_tree = NULL;
    loaded_tree = load_stats ( "test.stats" );
    printf("=================\n");
    print_stats_summary ( loaded_tree, ">" );
    print_patch_stats( stats_tree, tpl->k );
    save_stats ( "test2.stats", loaded_tree );
    // merge in place
    merge_stats ( loaded_tree, stats_tree, 1 );
    printf("=================\n");
    print_stats_summary ( loaded_tree, ">" );
    patch_node_t* merged_tree = merge_stats ( loaded_tree, stats_tree, 0 ); // not in place
    //
    // should yield everything doubled
    //
    print_stats_summary ( merged_tree, ">" );
    //
    // find neighbors
    //
    // by default, a patch is initialized to all-zeros
    printf ( "neighbors:\n" );
    patch_t* center;
    center = alloc_patch ( tpl->k );
    const index_t maxd = 2;
    neighbor_list_t neighbors = find_neighbors ( stats_tree, center, maxd );
    for ( int i = 0 ; i < neighbors.number ; ++i ) {
        get_leaf_patch ( center, neighbors.neighbors[ i ].patch_node );
        printf ( "dist %02ld ", neighbors.neighbors[ i ].dist );
        print_binary_patch ( center );
    }
    //
    // merge nodes with distance <= 2
    //
    printf ( "before merge\n" );
    print_patch_stats ( stats_tree, center->k );
    for ( int i = 0 ; i < center->k ; ++i )
        center->values[ i ] = 0;
    patch_node_t* dest = get_patch_node ( stats_tree, center );
    for ( int i = 0 ; i < neighbors.number ; ++i ) {
        if ( neighbors.neighbors[ i ].dist > 0 ) {
            patch_node_t* src = neighbors.neighbors[ i ].patch_node;
            merge_nodes ( dest, src );
        }
    }
    //
    // merge nodes
    //
    printf ( "after merge\n" );
    print_patch_stats ( stats_tree, center->k );

    free ( neighbors.neighbors );
    free_node ( merged_tree );
    free_node ( stats_tree );
    free_node ( loaded_tree );
    free_patch ( center );
    free_patch_template ( tpl );
    pixels_free ( img->pixels );
    free ( img );
    return 0;
}
