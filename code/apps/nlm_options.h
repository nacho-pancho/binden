#ifndef NL_OPTIONS_H
#define NL_OPTIONS_H

#include <argp.h>
#include "denoiser.h"

/**
 * Program options. These are filled in by the argument parser
 */
typedef struct nlm_config {
    const char * input_file;
    const char * output_file;
    const char * stats_file;
    const char * template_file;
    const char * prefiltered_file;
    int template_radius;
    int template_norm;
    int template_center;
    int template_scale;
    int search_radius;
    int max_dist;
    double perr;
    int decay;
    denoiser_f denoiser;
} nlm_config_t;

nlm_config_t parse_opt ( int argc, char* * argv );

#endif
