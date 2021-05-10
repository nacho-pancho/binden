#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <argp.h>

#include "image.h"
#include "ascmat.h"
#include "templates.h"
#include "logging.h"

/**
 * Program options. These are filled in by the argument parser
 */
typedef struct config {
    int template_radius;
    int template_norm;
    int template_center;
    int template_scale;
} config_t; 

/**
 * options handler
 */
static error_t parse_opt (int key, char *arg, struct argp_state *state);

/*---------------------------------------------------------------------------------------*/
/* MAIN */
/*---------------------------------------------------------------------------------------*/

int main (int argc, char *argv[]) {

    // defaults
    config_t cfg;
    cfg.template_radius = 4;
    cfg.template_norm = 2;
    cfg.template_center = 0 ;  
    cfg.template_scale  = 1 ; 

    // parse arguments
    struct argp_option options[] = {
      {"verbose",        'v', 0, OPTION_ARG_OPTIONAL, "Produce verbose output",0 },
      {"quiet",          'q', 0, OPTION_ARG_OPTIONAL, "Don't produce any output",0 },
      {"type",          't', "type",    0    , "'ball' or 'random'",0 },
      {"radius",        'r', "radius",  0    , "radius of the template ball",0 },
      {"norm",          'n', "norm",    0    , "norm of the template ball.",0 },
      {"scale",         's', "integer", 0    , "scale of the template ball.",0 },
      {"center",        'c', "bool",    0    , "include center in template.",0 },
      {"output",        'o', "file",    0    , "output file, defaults to stdout.",0 },
      { 0 } // terminator
    };
    const char program_doc[] = "\n*** create a patch template ***\n";
    const char args_doc[] = "";
    struct argp argp = { options, parse_opt, args_doc, program_doc, 0, 0, 0 };
    argp_parse (&argp, argc, argv, 0, 0, &cfg);

}

/*---------------------------------------------------------------------------------------*/


    
/*
 * argp callback for parsing a single option.
 */
static error_t parse_opt (int key, char *arg, struct argp_state *state) {
    /* Get the input argument from argp_parse,
     * which we know is a pointer to our arguments structure.
     */
    config_t *cfg = (config_t*) state->input;
    switch (key) {
    case 'q':
        set_log_level(LOG_ERROR);
        break;
    case 'v':
        set_log_level(LOG_DEBUG);
        break;
    case 'r':
        cfg->template_radius = atoi(arg);
        break;
    case 'n':
        cfg->template_norm = atoi(arg);
        break;
    case 's':
        cfg->template_scale = atoi(arg);
        break;
    case 'c':
        cfg->template_center = atoi(arg);
        break;

    case ARGP_KEY_ARG:
        error("Too many arguments!.\n");
        argp_usage (state);
        break;

    case ARGP_KEY_END:
        break;

    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}
