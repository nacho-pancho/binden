#ifndef patches_H
#define patches_H

#include "templates.h"
#include "image.h"

extern count_t num_ctx;

/**
 * patch realization
 */
typedef struct {
    val_t* values;
    unsigned int k;
} patch;

/**
 * Tree structure to efficiently store and search for patches
 */
typedef struct ctxtmp {
    count_t occu; // number of occurences of this patch
    char  leaf; // 1 if this node is a leaf
    struct ctxtmp** children;
    void* stats; // very generic!
} patch_node_t;

/**
 * Linear access to patches
 */
typedef struct {
    count_t nctx;
    patch_node_t** nodes;
} patch_list_t;

/**
 * Strategy for mapping patches
 */
typedef void (*patch_mapper_t)(const patch* orig_ctx, patch* mapped_ctx);

patch_node_t* create_node(const int leaf); // should be static

const patch_node_t* get_patch_node(const patch_node_t* ptree, const patch* pctx); // should be static

void destroy_node(patch_node_t* pnode); // should be static

void get_patch(const image_t* pimg, const template_t* ptpl, int i, int j, patch_mapper_t mapper, patch* pctx);

void get_linear_patch(const image_t* pimg, const linear_template_t* ptpl, int i, int j, patch_mapper_t mapper, patch* pctx);

void print_patch(const patch* pctx);

#endif