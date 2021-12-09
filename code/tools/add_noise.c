/**
 * Add Bernoulli Noise
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h> // for memcmp
#include <argp.h>

#include "logging.h"
#include "pnm.h"
#include "image.h"
/**
 * Program options. These are filled in by the argument parser
 */
typedef struct config {
    const char * input_file;
    const char * output_file;
    double p01;
    double p10;
    int seed;
} config_t;

config_t parse_opt ( int argc, char* * argv );

int main ( int argc, char* argv[] ) {

    image_t out;
    config_t cfg = parse_opt ( argc, argv );

    image_t* img = read_pnm ( cfg.input_file );

    if ( img == NULL ) {
        fprintf ( stderr, "error opening image %s.\n", cfg.input_file );
        return RESULT_ERROR;
    }
    if ( img->info.result != RESULT_OK ) {
        fprintf ( stderr, "error reading image %s.\n", cfg.input_file );
        pixels_free ( img->pixels );
        free ( img );
        return RESULT_ERROR;
    }
    if ( img->info.maxval > 1 ) {
        fprintf ( stderr, "only binary images supported.\n" );
        pixels_free ( img->pixels );
        free ( img );
        return RESULT_ERROR;
    }

    out.info = img->info;
    out.pixels = pixels_copy ( &img->info, img->pixels );
    const index_t n = img->info.width;
    const index_t m = img->info.height;
    const index_t npatches = m * n;
    //
    // if only p01 is specified (p10=-1), the noise is assumed symmetric and split even between p01 and p10
    //
    const double p01 = cfg.p10 > 0 ? cfg.p01 : cfg.p01/2.0;
    const double p10 = cfg.p10 > 0 ? cfg.p10 : p01;

    fprintf ( stdout, "adding noise with p01=%6.4f p10=%6.4f and seed=%d.\n", p01, p10, cfg.seed );
    srand48(cfg.seed);
    for ( int i = 0, li = 0 ; i < m ; ++i ) {
        for ( int j = 0 ; j < n ; ++j, ++li ) {
            const int x = get_linear_pixel(img, li);
            const double coin = drand48();
	    if (!x && (coin < p01)) { // 0->1
              set_linear_pixel ( &out, li, 1);
	    } else if (x && (coin < p10)) {
              set_linear_pixel ( &out, li, 0);
	    }
        }
    }

    //
    // second pass: using denoised contexts
    //
    //stats = gather_patch_stats ( img, pre, tpl, NULL, NULL );
    //apply_denoiser ( &out, img, perr, tpl->k, quorum_map, quorum_freq, quorum_freq_1 );

    int res = write_pnm ( cfg.output_file, &out );
    if ( res != RESULT_OK ) {
        fprintf ( stderr, "error writing image %s.\n", cfg.output_file );
    }


    pixels_free ( img->pixels );
    pixels_free ( out.pixels );
    free ( img );
    return res;
}



/**
 * These are the options that we can handle through the command line
 */
static struct argp_option options[] = {
    {"verbose",        'v', 0, OPTION_ARG_OPTIONAL, "Produce verbose output", 0 },
    {"quiet",          'q', 0, OPTION_ARG_OPTIONAL, "Don't produce any output", 0 },
    {"output",         'o', "file",    0, "output file", 0 },
    {"perr",           'p', "probability", 0, "symmetric error probability", 0 },
    {"pzero",          '0', "probability", 0, "error probability 0->1", 0 },
    {"pone",           '1', "probability", 0, "error probability 1->0", 0 },
    {"seed",           's', "seed",    0, "random seed.", 0 },
    { 0 } // terminator
};

/**
 * options handler
 */
static error_t _parse_opt ( int key, char * arg, struct argp_state * state );

/**
 * General description of what this program does; appears when calling with --help
 */
static char program_doc[] =
    "\n*** add noise to  images    ***\n";

/**
 * A general description of the input arguments we accept; appears when calling with --help
 */
static char args_doc[] = "<INPUT_FILE>";

/**
 * argp configuration structure
 */
static struct argp argp = { options, _parse_opt, args_doc, program_doc, 0, 0, 0 };

config_t parse_opt ( int argc, char* * argv ) {

    config_t cfg;
    cfg.input_file  = NULL;
    cfg.output_file = "denoised.pnm";
    cfg.p01 = 0.1;
    cfg.p10 = -1.0; // if left unspecified, p10 = p01
    cfg.seed = 42;
    argp_parse ( &argp, argc, argv, 0, 0, &cfg );

    return cfg;
}

/*
 * argp callback for parsing a single option.
 */
static error_t _parse_opt ( int key, char * arg, struct argp_state * state ) {
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
    case 'o':
        cfg->output_file = arg;
        break;
    case 's':
        cfg->seed = atoi ( arg );
        break;
    case 'p':
        cfg->p01 = atof ( arg ) / 2;
        cfg->p10 = cfg->p01;
        break;
    case '0':
        cfg->p01 = atof ( arg );
        break;
    case '1':
        cfg->p10 = atof ( arg );
        break;
    case 'r':
        cfg->p10 = atof ( arg );
        break;

    case ARGP_KEY_ARG:
        switch ( state->arg_num ) {
        case 0:
            cfg->input_file = arg;
            break;
        default:
            /** too many arguments! */
            error ( "Too many arguments!.\n" );
            argp_usage ( state );
            break;
        }
        break;
    case ARGP_KEY_END:
        if ( state->arg_num < 1 ) {
            /* Not enough mandatory arguments! */
            error ( "Too FEW arguments!\n" );
            argp_usage ( state );
        }
        break;

    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}
