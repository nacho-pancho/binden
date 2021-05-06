#include <stdio.h>
#include <stdlib.h>

#include "pnm.h"
#include "image.h"
#include "templates.h"
#include "patches.h"
#include "stats.h"

int main(int argc, char* argv[]) {

    if (argc < 2) { 
        fprintf(stderr,"usage: %s <image>.\n",argv[0]); 
        return RESULT_ERROR; 
    }
    const char* fname = argv[1];    
    image_t* img = read_pnm(fname);
    if (img == NULL) {
        fprintf(stderr,"error opening image %s.\n",fname);
        return RESULT_ERROR;
    }
    if (img->info.result != RESULT_OK) {
        fprintf(stderr,"error reading image %s.\n",fname);
        pixels_free(img->pixels);
        free(img);
        return RESULT_ERROR;
    }
    //
    //
    //
    //
    // create template  
    //
    const int radius = 2;
    const int norm = 1;
    const int exclude_center = 1;
    patch_template_t* tpl;
    
    tpl = generate_ball_template(radius,norm,exclude_center);
    //
    // coordinate access
    //
    patch_node_t* stats_tree;
    printf("image alphabet size %d\n",img->info.maxval + 1);
    stats_tree = gather_patch_stats(img,img,tpl,NULL,NULL);
    printf("number of pixels %d\n",img->info.width*img->info.height);
    print_stats_summary(stats_tree,">");
    save_stats("test.stats",stats_tree);
    patch_node_t* loaded_tree = NULL;
    loaded_tree = load_stats("test.stats");
    print_stats_summary(loaded_tree,">");
    save_stats("test2.stats",loaded_tree);
    // merge in place
    merge_stats(loaded_tree,stats_tree,1);
    print_stats_summary(loaded_tree,">");
    patch_node_t* merged_tree = merge_stats(loaded_tree,stats_tree,0); // not in place
    //
    // should yield everything doubled
    //
    print_stats_summary(merged_tree,">");
    //
    // find neighbors
    //
    // by default, a patch is initialized to all-zeros    
    patch_t* center;    
    center = alloc_patch(tpl->k);
    const index_t maxd = 2;
    const index_t maxn = 256; // combinations: 16*15 ~ 256
    neighbor_list_t neighbors = find_neighbors ( stats_tree, center, maxd);
    for (int i = 0; i < neighbors.number; ++i) {
        get_leaf_patch(center,neighbors.neighbors[i].patch_node);
        printf("dist %02ld ",neighbors.neighbors[i].dist);
        print_binary_patch(center);        
    }
    //
    // merge nodes with distance <= 2
    //
    printf("before merge\n");
    char line[1024];
    line[0] = 0;
    print_patch_stats(stats_tree,line);
    for (int i = 0; i < center->k; ++i)
        center->values[i] = 0;
    patch_node_t* dest = get_patch_node(stats_tree, center);
    for (int i = 0; i < neighbors.number; ++i) {
        if (neighbors.neighbors[i].dist > 0) {
            patch_node_t* src = neighbors.neighbors[i].patch_node;            
            merge_nodes(dest,src);
        }
    }
    //
    // merge nodes
    //
    printf("after merge\n");
    line[0] = 0;
    print_patch_stats(stats_tree,line);

    free(neighbors.neighbors);
    free_node(merged_tree);
    free_node(stats_tree);
    free_node(loaded_tree);
    free_patch(center);
    free_patch_template(tpl);
    pixels_free(img->pixels);
    free(img);
    return 0;
}
