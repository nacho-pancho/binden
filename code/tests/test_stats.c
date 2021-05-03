#include <stdio.h>
#include <stdlib.h>

#include "pnm.h"
#include "image.h"
#include "templates.h"
#include "patches.h"
#include "stats.h"

int main(int argc, char* argv[]) {
    char line[1024];

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
    const int radius = 3;
    const int norm = 2;
    const int exclude_center = 1;
    patch_template_t* tpl;
    patch_t* pat;
    
    tpl = generate_ball_template(radius,norm,exclude_center);
    pat = alloc_patch(tpl->k);
    //
    // coordinate access
    //
    patch_node_t* stats_tree;
    printf("image alphabet size %d\n",img->info.maxval + 1);
    stats_tree = gather_patch_stats(img,img,tpl,NULL,NULL);
    //line[0] = 0;
    //print_patch_stats(stats_tree,line);
    printf("number of pixels %d\n",img->info.width*img->info.height);
    summarize_patch_stats(stats_tree,">");
    save_stats("test.stats",stats_tree);
    patch_node_t* loaded_tree = NULL;
    loaded_tree = load_stats("test.stats");
    summarize_patch_stats(loaded_tree,">");
    save_stats("test2.stats",loaded_tree);
    // merge in place
    merge_stats(loaded_tree,stats_tree,1);
    summarize_patch_stats(loaded_tree,">");
    patch_node_t* merged_tree = merge_stats(loaded_tree,stats_tree,0); // not in place
    //
    // should yield everything doubled
    //
    summarize_patch_stats(merged_tree,">");
    free_node(merged_tree);
    free_node(stats_tree);
    free_node(loaded_tree);
    free_patch(pat);
    free_patch_template(tpl);
    pixels_free(img->pixels);
    free(img);
    return 0;
}
