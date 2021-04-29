
#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <stdio.h>
#include "types.h"

/** 2D coordinates */
typedef struct coord {
    short i;
    short j;
} coord_t;

/** set of relative 2D coordinates */
typedef struct template {
    coord_t* coords;
    int k;
} template_t;

/** Linear version of a template for faster access */
typedef struct linear_template {
    int* li;
    int k;
} linear_template_t;


template_t* alloc_template(int maxk);

linear_template_t* alloc_linear_template(int maxk);

void free_template(template_t* pt);

void free_linear_template(linear_template_t* pt);

template_t* read_template(const char* fname);


template_t* generate_uniform_random_template(int max_l1_radius, int k, int exclude_center);

template_t* generate_random_template(int radius, int norm, int k, int sym, int exclude_center);

template_t* generate_ball_template(int radius, int norm, int exclude_center);

linear_template_t* linearize_template(const template_t* pt, int nrows, int ncols);

template_t* symmetrize_template(const template_t* in);

void print_template(const template_t* ptpl);

void dump_template(const template_t* ptpl,FILE* ft);

void read_template_multi(const char* fname, template_t** ptpls, int* ntemplates);

template_t* sort_template(template_t* orig, int in_place);

#endif
