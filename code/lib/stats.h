#ifndef STATS_H
#define STATS_H
#include "patches.h"

#define ALPHA 2
/**
 * Tree structure to efficiently store and search for patches
 */
typedef struct patch_node {
    struct patch_node* children[ALPHA];
    struct patch_node* parent; 
    pixel_t value; // patch sample value corresponding to this node
    index_t occu; // number of occurences of this node
    index_t counts; // number of 1s
    char leaf;  // 1 if this node is a leaf
} patch_node_t;


typedef struct neighbor {
    patch_node_t* patch_node;
    index_t dist;
} neighbor_t;


typedef struct neighbor_list {
    neighbor_t* neighbors;
    index_t number;
} neighbor_list_t;

/**
 * Flat structure to efficiently store and search for patches
 */

void free_node ( patch_node_t * node );

void delete_node ( patch_node_t * node);

void merge_nodes ( patch_node_t * dest, patch_node_t* src);

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

/*---------------------------------------------------------------------------------------*/

index_t get_patch_stats ( const patch_node_t * ptree, const patch_t * pctx );

/*---------------------------------------------------------------------------------------*/

patch_node_t* get_patch_node ( patch_node_t * ptree, patch_t * pctx );

/*---------------------------------------------------------------------------------------*/

const patch_node_t* get_patch_node_const ( const patch_node_t * ptree, const patch_t * pctx );

/*---------------------------------------------------------------------------------------*/

neighbor_list_t find_neighbors ( 
    patch_node_t* ptree, 
    const patch_t* center, 
    const index_t maxd, 
    const index_t maxn);

/*---------------------------------------------------------------------------------------*/

/**
 * fill target patch with the samples corresponding to the specified leaf
 */ 
void get_leaf_patch ( patch_t * pctx, const patch_node_t * leaf );

/*---------------------------------------------------------------------------------------*/
/**
 * Print a patch tree with its counts
 */
void print_patch_stats ( patch_node_t * pnode, char * prefix );

/*---------------------------------------------------------------------------------------*/

void summarize_stats(patch_node_t * pnode, index_t* nleaves, index_t* totoccu, index_t* totcount);

/*---------------------------------------------------------------------------------------*/

/**
 * Print a patch tree with its counts
 */
void print_stats_summary ( patch_node_t * pnode, const char * prefix );

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


#endif
