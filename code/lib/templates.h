
#ifndef TEMPLATE_H
#define TEMPLATE_H

#include "types.h"

/** 2D template_t */
typedef struct {
    index_t* is;
    index_t* js;
    unsigned int k;
} template_t;

/** Linear version of a template for faster access */
typedef struct {
    index_t* li;
    unsigned int k;
} linear_template_t;

void read_template(const char* fname, template_t* ptpl);

void print_template(const template_t* ptpl);

void read_template_multi(const char* fname, template_t** ptpls, int* ntemplates);

template_t* ini_template(template_t* pt, int k);

void destroy_template(template_t* pt);

template_t* generate_random_template(int max_l1_radius, int k, template_t* pt);

void linearize_template(const template_t* pt, int nrows, int ncols, const linear_template_t* plt);


#endif
