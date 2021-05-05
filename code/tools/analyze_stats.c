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
#include <assert.h>

#include "stats.h"                 
#include "logging.h"
#include "patches.h"

void scan_tree(patch_node_t* node, patch_node_t** node_list, index_t* pos) {
    if (node->leaf) {
        node_list[(*pos)++] = node;
    } else {
        for (int i = 0; i < ALPHA; ++i)
            scan_tree(node->children[i],node_list,pos);
    }
}

int ascending = 0;

static int compare_nodes_occu(const void* va, const void* vb) {
    const patch_node_t* a = *((const patch_node_t**) va);
    const patch_node_t* b = *((const patch_node_t**) vb);
    const long res = ascending ? (a->occu - b->occu) : (b->occu - a->occu);
    return res > 0 ? 1: (res < 0 ? -1: 0);
}

static void print_node_list(patch_node_t** node_list, const index_t nnodes, const index_t totoccu, patch_t* aux ) {
    index_t accu = 0;
    for (int i = 0; i < nnodes; ++i) {
        printf("%06d | ",i);
        get_leaf_patch(aux,node_list[i]);
        for (int j = 0; j < aux->k; ++j) {
            putchar('0'+aux->values[j]);
        }
        const patch_node_t* p = node_list[i];
        const double P = ((double) p->counts) / ((double) p->occu);
        const double Q = ((double) p->occu) / ((double) totoccu);
        accu += p->occu;
        const double F = ((double) accu) / ((double) totoccu);
        printf(" | %12ld | %12.10f | %12.10f | %12ld | %12.10f\n", p->occu, Q, F, p->counts, P);
    }
}
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
     * load stats
     */
    patch_node_t* stats_tree = load_stats(cfg.stats_file);
    if (!stats_tree) {
        error("Could not open file %s for reading.\n",cfg.stats_file);
        exit(1);
    }
    //
    // scan 
    //
    index_t nleaves = 0, totoccu = 0, totcount = 0;
    summarize_stats(stats_tree,&nleaves,&totoccu,&totcount);
    //
    // allocate analysis structure
    //
    index_t pos = 0;
    patch_node_t** node_list = (patch_node_t**) calloc(nleaves,sizeof(patch_node_t*));
    scan_tree(stats_tree,node_list,&pos);
    printf(" pos %ld nleaves %ld\n",pos,nleaves);
    if (nleaves != pos) {
        exit(1);
    }
    patch_t* aux = alloc_patch(template->k);
    printf("sorting list\n");
    qsort(node_list,nleaves,sizeof(patch_node_t*),compare_nodes_occu);
    printf("printing list\n");
    print_node_list(node_list,nleaves,totoccu,aux);
    //
    // cleanup and go
    //
    printf("cleanup\n");
    free_patch(aux);
    free(node_list);
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
