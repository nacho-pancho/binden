#ifndef STATS_H
#define STATS_H
#include "patches.h"

#define ALPHA 2
/**
 * Tree structure to efficiently store and search for patches
 */
typedef struct patch_node {
    index_t occu; // number of occurences of this node
    char leaf;  // 1 if this node is a leaf
    struct patch_node* children[ALPHA];
    index_t counts; // number of 1s
} patch_node_t;


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
                                    const patch_template_t * ptpl,
                                    patch_mapper_t mapper,
                                    patch_node_t * ptree );


index_t get_patch_stats ( const patch_node_t * ptree, const patch_t * pctx );

/*---------------------------------------------------------------------------------------*/
/**
 * Print a patch tree with its counts
 */
void print_patch_stats ( patch_node_t * pnode, char * prefix );

/*---------------------------------------------------------------------------------------*/
/**
 * Print a patch tree with its counts
 */
void summarize_patch_stats ( patch_node_t * pnode, const char * prefix );

/*---------------------------------------------------------------------------------------*/
/**
 * load stats from custom formatted file
 */
patch_node_t * load_stats ( const char * fname );

/*---------------------------------------------------------------------------------------*/

/**
 * save stats to custom formatted file
 */
int save_stats ( const char * fname, const patch_node_t * stats );

/*---------------------------------------------------------------------------------------*/

patch_node_t * merge_stats ( patch_node_t* dest, const patch_node_t * src, const int in_place );

/*---------------------------------------------------------------------------------------*/

void free_stats ( patch_node_t * pnode );

/*---------------------------------------------------------------------------------------*/

#endif
