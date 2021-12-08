#ifndef STATS_H
#define STATS_H
#include "patches.h"

#define ALPHA 2
/**
 * Tree structure to efficiently store and search for patches
 */
typedef struct patch_node {
    struct patch_node* children[ ALPHA ];
    struct patch_node* parent;
    pixel_t value; // patch sample value corresponding to this node
    index_t occu; // number of occurences of this node
    index_t counts; // number of 1s
    // not null if this is a cluster center:
    // each value indicates how many times the corresponding element
    // was found to be different from the same value in the center of the cluster
    // in other points that now belong to this cluster
    index_t* diff;
    char leaf;  // 1 if this node is a leaf
} patch_node_t;


typedef struct neighbor {
    patch_node_t* patch_node;
    index_t dist;
} neighbor_t;


typedef struct neighbor_list {
    neighbor_t* neighbors;
    index_t number;
    index_t maxnumber;
} neighbor_list_t;

/**
 * Flat structure to efficiently store and search for patches
 */

void free_node ( patch_node_t * node );

void delete_node ( patch_node_t * node );

void merge_nodes ( patch_node_t * dest, patch_node_t* src );

void flatten_stats ( patch_node_t* node, patch_node_t* * node_list, index_t* pos );

void sort_stats ( patch_node_t* * node_list, index_t nnodes );

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

patch_node_t * get_patch_node ( patch_node_t * ptree, patch_t * pctx );

/*---------------------------------------------------------------------------------------*/

const patch_node_t * get_patch_node_const ( const patch_node_t * ptree, const patch_t * pctx );

/*---------------------------------------------------------------------------------------*/

neighbor_list_t find_neighbors (
    patch_node_t* ptree,
    const patch_t* center,
    const index_t maxd );

/*---------------------------------------------------------------------------------------*/

/**
 * fill target patch with the samples corresponding to the specified leaf
 */
void get_leaf_patch ( patch_t * pctx, const patch_node_t * leaf );

/*---------------------------------------------------------------------------------------*/
/**
 * Print a patch tree with its counts
 */
void print_patch_stats ( patch_node_t * pnode, index_t k );

/*---------------------------------------------------------------------------------------*/

void print_node_list ( patch_node_t* * node_list, const index_t nnodes, const index_t totoccu, patch_t* aux );

/*---------------------------------------------------------------------------------------*/

void summarize_stats ( patch_node_t * pnode, index_t* nleaves, index_t* totoccu, index_t* totcount );

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

/** strategy for pruning a node */
typedef int (*prune_decision_f)(const patch_node_t *base, const patch_node_t* left, const patch_node_t* right, void* par);

/** prune a stats tree using some strategy */
patch_node_t * prune_stats ( patch_node_t* dest, prune_decision_f prune_decision, void* prune_par, const int in_place );

/*---------------------------------------------------------------------------------------*/

void free_stats ( patch_node_t * pnode );

/*---------------------------------------------------------------------------------------*/

patch_node_t * cluster_stats (
    patch_node_t * in,
    const index_t K,
    const index_t maxd,
    const index_t minoccu,
    const index_t maxclusters );

/*---------------------------------------------------------------------------------------*/

void test_stats_iter ( index_t k, patch_node_t* tree );

#endif
