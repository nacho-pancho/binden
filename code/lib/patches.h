#ifndef patches_H
#define patches_H

#include "templates.h"
#include "image.h"

/**
 * patch realization
 */
typedef struct patch {
    val_t * values;
    int k;
} patch_t;


/**
 * must be set before using this module
 */
void set_context_alphabet_size ( int sz );

/**
 * Strategy for transforming patches to another representation on the fly
 */
typedef void ( * patch_mapper_t )( const patch_t * orig_patch, patch_t * mapped_patch );

patch_t * alloc_patch ( int k );

void free_patch ( patch_t * p );

void get_patch ( const image_t * pimg, const template_t * ptpl, int i, int j, patch_mapper_t mapper, patch_t * ppatch );

void get_linear_patch ( const image_t * pimg, const linear_template_t * ptpl, int i, int j, patch_mapper_t mapper, patch_t * ppatch );

void print_patch ( const patch_t * ppatch );

#endif
