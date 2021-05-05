#include <stdlib.h>

#include "nlm_options.h"
#include "logging.h"
/**
 * These are the options that we can handle through the command line
 */
static struct argp_option options[] = {
    {"verbose",        'v', 0, OPTION_ARG_OPTIONAL, "Produce verbose output",0 },
    {"quiet",          'q', 0, OPTION_ARG_OPTIONAL, "Don't produce any output",0 },
    {"output",         'o', 0, 0                  , "output file",0 },
    {"tradius",        'r', 0, 0                  , "radius of the template ball",0 },
    {"tnorm",          'n', 0, 0                  , "norm of the template ball.",0 },
    {"tscale",         's', 0, 0                  , "scale of the template ball.",0 },
    {"tcenter",        'c', 0, 0                  , "include center in template.",0 },
    {"maxdist",        'd', 0, 0                  , "output file",0 },
    {"perr",           'p', 0, 0                  , "output file",0 },
    {"search",         'R', 0, 0                  , "output file",0 },
    { 0 } // terminator
};

/**
 * options handler
 */
static error_t _parse_opt (int key, char *arg, struct argp_state *state);

/**
 * General description of what this program does; appears when calling with --help
 */
static char program_doc[] =
    "\n*** gather statistics from a list of images    ***\n";

/**
 * A general description of the input arguments we accept; appears when calling with --help
 */
static char args_doc[] = "<STATS_FILE> <TEMPLATE_FILE>";

/**
 * argp configuration structure
 */
static struct argp argp = { options, _parse_opt, args_doc, program_doc, 0, 0, 0 };

nlm_config_t parse_opt(int argc, char** argv ) {

    nlm_config_t cfg;
    cfg.input_file  = "";
    cfg.output_file = "nlm_out.pnm";
    cfg.template_radius = 4;
    cfg.template_norm = 2;
    cfg.template_center = 0 ; // exclude center
    cfg.template_scale  = 1 ; // exclude center
    cfg.search_radius = 40;
    cfg.max_dist = 10;
    cfg.perr = 0.1;

    argp_parse (&argp, argc, argv, 0, 0, &cfg);
    
    return cfg;
}

/*
 * argp callback for parsing a single option.
 */
static error_t _parse_opt (int key, char *arg, struct argp_state *state) {
    /* Get the input argument from argp_parse,
     * which we know is a pointer to our arguments structure.
     */
    nlm_config_t *cfg = (nlm_config_t*) state->input;
    switch (key) {
    case 'q':
        set_log_level(LOG_ERROR);
        break;
    case 'v':
        set_log_level(LOG_DEBUG);
        break;
    case 'o':
        cfg->output_file = arg;
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
    case 'd':
        cfg->max_dist = atoi(arg);
        break;
    case 'p':
        cfg->perr = atof(arg);
        break;
    case 'R':
        cfg->search_radius= atoi(arg);
        break;

    case ARGP_KEY_ARG:
        switch (state->arg_num) {
        case 0:
            cfg->input_file = arg;
            break;
        default:
            /** too many arguments! */
            error("Too many arguments!.\n");
            argp_usage (state);
            break;
        }
        break;
    case ARGP_KEY_END:
        if (state->arg_num < 1) {
            /* Not enough mandatory arguments! */
            error("Too FEW arguments!\n");
            argp_usage (state);
        }
        break;

    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}
