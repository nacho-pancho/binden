#ifndef NL_OPTIONS_H
#define NL_OPTIONS_H

#include <argp.h>
#include "denoiser.h"

/**
 * Program options. These are filled in by the argument parser
 */
typedef struct config {
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
    int max_clusters;
    int max_dist;
    double p01;
    double p10;
    double nlm_window_scale;
    double nlm_weight_scale;
    int decay;
    int seed;
    int verbose;
    denoiser_f denoiser;
} config_t;

config_t parse_opt ( int argc, char* * argv );

#endif
