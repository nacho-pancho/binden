/**
 * \file gather_stats.c
 * \brief Gather patch statistics from a bunch of images
 *
 *  Uses GNU argp to parse arguments
 *  see http://www.gnu.org/software/libc/manual/html_node/Argp.html
 */
#include <stdlib.h>
#include <time.h>                     // basic benchmarking
#include <argp.h>                     // argument parsing
#include <string.h>

#include "stats.h"                 
#include "logging.h"

/**
 * These are the options that we can handle through the command line
 */
static struct argp_option options[] = {
    {"verbose",        'v', 0, OPTION_ARG_OPTIONAL, "Produce verbose output",0 },
    {"quiet",          'q', 0, OPTION_ARG_OPTIONAL, "Don't produce any output",0 },
    { 0 } // terminator
};

/**
 * Program options. These are filled in by the argument parser
 */
typedef struct  {
    char *stats_file;   
    char *template_file; 
} config_st; 

/**
 * options handler
 */
static error_t parse_opt (int key, char *arg, struct argp_state *state);

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
static struct argp argp = { options, parse_opt, args_doc, program_doc, 0, 0, 0 };

/**
 * main function
 */
int main(int argc, char **argv) {
    config_st cfg; // command-line program configuration
    time_t t0, t1;      // for benchmarking
    t0 = clock();

    info("Setting default configuration...\n");

    /*
     * Default program configuration
     */
    set_log_level(LOG_INFO);
    set_log_stream(stdout);
    cfg.stats_file = NULL;
    cfg.template_file = NULL; 
    info("Parsing arguments...\n");
    /*
     * call parser
     */
    argp_parse (&argp, argc, argv, 0, 0, &cfg);
    /*
     * load template
     */
    patch_template_t* template;
    template = read_template(cfg.template_file);
    if (template->k == 0) {
        error("Invalid template or missing template file.\n");
        exit(1);
    }
    /*
     * sort by distence to center
     */
    sort_template(template,1);
    print_template(template);
    /*
     * run stuff
     */
    patch_node_t* stats_tree = load_stats(cfg.stats_file);
    if (!stats_tree) {
        error("Could not open file %s for reading.\n",cfg.stats_file);
        exit(1);
    }
    free_patch_template(template);
    free_stats(stats_tree);
    t1 = clock();
    printf("Took %ld seconds.\n",t1-t0);
    exit (0);
}


/*
 * argp callback for parsing a single option.
 */
static error_t parse_opt (int key, char *arg, struct argp_state *state) {
    /* Get the input argument from argp_parse,
     * which we know is a pointer to our arguments structure.
     */
    config_st *cfg = (config_st*) state->input;
    switch (key) {
    case 'q':
        set_log_level(LOG_ERROR);
        break;
    case 'v':
        set_log_level(LOG_DEBUG);
        break;

    case ARGP_KEY_ARG:
        switch (state->arg_num) {
        case 0:
            cfg->stats_file = arg;
            break;
        case 1:
            cfg->template_file = arg;
            break;
        default:
            /** too many arguments! */
            error("Too many arguments!.\n");
            argp_usage (state);
            break;
        }
        break;
    case ARGP_KEY_END:
        if (state->arg_num < 2) {
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
