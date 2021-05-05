#ifndef NL_OPTIONS_H
#define NL_OPTIONS_H

#include <argp.h>

/**
 * Program options. These are filled in by the argument parser
 */
typedef struct  {
    const char *input_file;
    const char *output_file;
    int template_radius;
    int template_norm;
    int template_center;
    int template_scale;
    int search_radius;
    int max_dist;
    double perr;
} nlm_config_t; 

nlm_config_t parse_opt(int argc, char** argv );

#endif