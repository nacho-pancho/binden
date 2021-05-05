#ifndef patches_H
#define patches_H

#include "image.h"
#include "templates.h"
/**
 * patch realization
 */
typedef struct patch {
    pixel_t * values;
    int k;
} patch_t;

/**
 * Strategy for transforming patches to another representation on the fly
 */
typedef void ( * patch_mapper_t )( const patch_t * orig_patch, patch_t * mapped_patch );

/**
 * must be set before using this module
 */
void set_context_alphabet_size ( int sz );

patch_t * alloc_patch ( int k );

void free_patch ( patch_t * p );

void get_patch ( const image_t * pimg, const patch_template_t * ptpl, int i, int j, patch_t * ppatch );

void get_mapped_patch ( const image_t * pimg, const patch_template_t * ptpl, 
    int i, int j, patch_mapper_t mapper, patch_t * ppatch, patch_t* mapped );

void get_linear_patch ( const image_t * pimg, const linear_template_t * ptpl, 
    int i, int j, patch_t * ppatch );

void get_mapped_linear_patch ( const image_t * pimg, const linear_template_t * ptpl, 
    int i, int j, patch_mapper_t mapper, patch_t * ppatch, patch_t * mapped );

void print_patch ( const patch_t * ppatch );

void print_binary_patch ( const patch_t * ppatch );

void print_patch_fancy ( const patch_t* patch, const patch_template_t * ptpl );

#endif
