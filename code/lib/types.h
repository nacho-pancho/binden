#ifndef TYPES_H
#define TYPES_H

typedef short val_t;
typedef int index_t;
typedef unsigned long count_t;

/** A basic matrix */
typedef struct {
    double* values; /* MxN, in C order */
    unsigned int nrows;
    unsigned int ncols;
} Matrix;

/** A basic vector */
typedef struct {
    double* values;
    unsigned int n;
} Vector;

#endif
