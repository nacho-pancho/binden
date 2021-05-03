
#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <stdio.h>
#include "types.h"

/** 2D coordinates */
typedef struct {
    index_t i;
    index_t j;
} coord_t;

/** set of relative 2D coordinates */
typedef struct {
    coord_t * coords;
    index_t k;
} patch_template_t;

/** Linear version of a template for faster access */
typedef struct {
    index_t * li;
    index_t k;
} linear_template_t;


patch_template_t * alloc_patch_template ( index_t maxk );

linear_template_t * alloc_linear_template ( index_t maxk );

void free_patch_template ( patch_template_t * pt );

void free_linear_template ( linear_template_t * pt );

patch_template_t * read_template ( const char * fname );


patch_template_t * generate_uniform_random_template ( index_t max_l1_radius, index_t k, index_t exclude_center );

patch_template_t * generate_random_template ( index_t radius, index_t norm, index_t k, index_t sym, index_t exclude_center );

patch_template_t * generate_ball_template ( index_t radius, index_t norm, index_t exclude_center );

patch_template_t * dilate_template ( patch_template_t* in, index_t scale, index_t in_place);

linear_template_t * linearize_template ( const patch_template_t * pt, index_t nrows, index_t ncols );

patch_template_t * symmetrize_template ( const patch_template_t * in );

void print_template ( const patch_template_t * ptpl );

void dump_template ( const patch_template_t * ptpl, FILE * ft );

void read_template_multi ( const char * fname, patch_template_t * * ptpls, index_t * ntemplates );

patch_template_t * sort_template ( patch_template_t * orig, index_t in_place );

#endif
