#ifndef patches_H
#define patches_H

#include "templates.h"
#include "image.h"


/**
 * patch realization
 */
typedef struct patch {
    val_t* values;
    unsigned int k;
} patch_t;

/**
 * Tree structure to efficiently store and search for patches
 */
typedef struct patch_node {
    count_t occu; // number of occurences of this patch
    char  leaf; // 1 if this node is a leaf
    struct patch_node** children;
    count_t nchildren;
    void* stats; // very generic!
} patch_node_t;

/**
 * Linear access to patches
 */
typedef struct {
    count_t npatch;
    patch_node_t** nodes;
} patch_list_t;

/**
 * must be set before using this module
 */
void set_context_alphabet_size(int sz);

/**
 * Strategy for mapping patches
 */
typedef void (*patch_mapper_t)(const patch_t* orig_patch, patch_t* mapped_patch);

patch_node_t* create_node(const int leaf, const int alph_size); // should be static

const patch_node_t* get_patch_node(const patch_node_t* ptree, const patch_t* ppatch); // should be static

void destroy_node(patch_node_t* pnode); // should be static

void get_patch(const image_t* pimg, const template_t* ptpl, int i, int j, patch_mapper_t mapper, patch_t* ppatch);

void get_linear_patch(const image_t* pimg, const linear_template_t* ptpl, int i, int j, patch_mapper_t mapper, patch_t* ppatch);

void print_patch(const patch_t* ppatch);

#endif