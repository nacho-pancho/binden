#ifndef STATS_H
#define STATS_H
#include "patches.h"

/**
 * Tree structure to efficiently store and search for patches
 */
typedef struct patch_node {
    count_t occu; // number of occurences of this node
    char leaf;  // 1 if this node is a leaf
    struct patch_node * * children;
    count_t nchildren;
    count_t counts;
} patch_node_t;


/**
 * Linear access to patches
 */
typedef struct {
    count_t npatch;
    patch_node_t * * nodes;
} patch_list_t;


void free_node ( patch_node_t * node );

/*
 * Gather patch statistics from an image, given a template
 *
 * @param pnoisy Noisy (input) image
 * @param patch_t template. This one is used to determine the patch in which each pixel occurs
 * @param M Alphabet size.
 * @param[out] tree This tree is populated with the patches that actually occur, their counts, and their center pixel histogram
 */
patch_node_t * gather_patch_stats ( const image_t * pnoisy,
                                    const image_t * pctx,
                                    const template_t * ptpl,
                                    patch_mapper_t mapper,
                                    patch_node_t * ptree );


count_t get_patch_stats ( const patch_node_t * ptree, const patch_t * pctx );

/**
 * Print a patch tree with its counts
 */
void print_patch_stats ( patch_node_t * pnode, char * prefix );

/*---------------------------------------------------------------------------------------*/
/**
 * load stats from custom formatted file
 */
patch_node_t * load_stats ( const char * fname );

/*---------------------------------------------------------------------------------------*/

/**
 * save stats to custom formatted file
 */
void save_stats ( const char * fname, const patch_node_t * stats );

#endif
