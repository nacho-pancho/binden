#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <argp.h>

#include "templates.h"
#include "logging.h"

#define FULL_TEMPLATE 0
#define BALL_TEMPLATE 1
#define RANDOM_TEMPLATE 2

/**
 * Program options. These are filled in by the argument parser
 */
typedef struct config {
    int radius;
    int norm;
    int center;
    int scale;
    int type;
    int size;
    int symmetric;
    int sorted;
    const char* output_file;
} config_t;

/**
 * options handler
 */
static error_t parse_opt ( int key, char * arg, struct argp_state * state );

/*---------------------------------------------------------------------------------------*/
/* MAIN */
/*---------------------------------------------------------------------------------------*/

int main ( int argc, char * argv[] ) {

    // defaults
    config_t cfg;
    cfg.type = FULL_TEMPLATE;
    cfg.radius = 4;
    cfg.norm   = 2;
    cfg.center = 0;
    cfg.scale  = 1;
    cfg.sorted = 0;
    cfg.symmetric = 1;
    cfg.size = 0; // must be specified for random templates
    cfg.output_file = "out.tpl";

    // parse arguments
    struct argp_option options[] = {
        {"verbose",       'v', 0, OPTION_ARG_OPTIONAL, "Produce verbose output", 0 },
        {"quiet",         'q', 0, OPTION_ARG_OPTIONAL, "Don't produce any output", 0 },
        {"type",          't', "type",    0, "'full', 'ball' or 'random'", 0 },
        {"size",          'k', "integer", 0, "number of pixels in template (for random templates)", 0 },
        {"radius",        'r', "radius",  0, "radius of the template", 0 },
        {"norm",          'n', "norm",    0, "norm of the template (for ball and random).", 0 },
        {"scale",         's', "integer", 0, "scale of the template.", 0 },
        {"center",        'c', 0, OPTION_ARG_OPTIONAL, "include center in template.", 0 },
        {"output",        'o', "file",    0, "output file, defaults to stdout.", 0 },
        {"sorted",        'S', 0,    OPTION_ARG_OPTIONAL, "sort template by L2 distance to center.", 0 },
        {"asymmetric",    'a', 0,    OPTION_ARG_OPTIONAL, "allow for asymmetric patches (for random templates).", 0 },
        { 0 } // terminator
    };
    const char program_doc[] = "\n*** create a patch template ***\n";
    const char args_doc[] = "";
    struct argp argp = { options, parse_opt, args_doc, program_doc, 0, 0, 0 };
    argp_parse ( &argp, argc, argv, 0, 0, &cfg );

    patch_template_t* tpl = NULL;
    if (cfg.type == FULL_TEMPLATE) {
        tpl = generate_ball_template(cfg.radius,1000,cfg.center);
    } else if (cfg.type == BALL_TEMPLATE) {
        tpl = generate_ball_template(cfg.radius,cfg.norm,cfg.center);
    } else if (cfg.type == RANDOM_TEMPLATE) {
        tpl = generate_random_template(cfg.radius,cfg.norm,cfg.size,cfg.symmetric,cfg.center);
    }
    if (cfg.sorted) {
        sort_template(tpl,1);
    }  
    FILE* ft = fopen(cfg.output_file,"w");
    if (!ft) {
        error ( "Unable to write to %s\n",cfg.output_file);
        free_patch_template(tpl);
        exit(1);
    }
    print_template(tpl);
    dump_template(tpl,ft);
    fclose(ft);
    free_patch_template(tpl);
    exit(0);
}

/*---------------------------------------------------------------------------------------*/



/*
 * argp callback for parsing a single option.
 */
static error_t parse_opt ( int key, char * arg, struct argp_state * state ) {
    /* Get the input argument from argp_parse,
     * which we know is a pointer to our arguments structure.
     */
    config_t * cfg = ( config_t* ) state->input;
    switch ( key ) {
    case 'q':
        set_log_level ( LOG_ERROR );
        break;
    case 'v':
        set_log_level ( LOG_DEBUG );
        break;
    case 'r':
        cfg->radius = atoi ( arg );
        break;
    case 'k':
        cfg->size = atoi ( arg );
        break;
    case 'a':
        cfg->symmetric = 0;
        break;
    case 'n':
        cfg->norm = atoi ( arg );
        break;
    case 's':
        cfg->scale = atoi ( arg );
        break;
    case 'c':
        cfg->center = atoi ( arg );
        break;
    case 'S':
        cfg->sorted = 1;
        break;
    case 'o':
        cfg->output_file = arg;
        break;
    case 't':
        if (!strncasecmp(arg,"full",4)) {
            cfg->type = FULL_TEMPLATE;
        } else if (!strncasecmp(arg,"ball",4)) {
            cfg->type = BALL_TEMPLATE;
        } else if (!strncasecmp(arg,"rand",4)) {
            cfg->type = RANDOM_TEMPLATE;
        } else {
            error ( "unknown template type: %s\n", arg);
        }
        break;

    case ARGP_KEY_ARG:
        error ( "Too many arguments!.\n" );
        argp_usage ( state );
        break;

    case ARGP_KEY_END:
        break;

    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}
