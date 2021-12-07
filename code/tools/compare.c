/**
 * Compare two binary images
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
    const char * input_file_1;
    const char * input_file_2;
    const char * output_file;
    double perr;
    int seed;
} config_t;

config_t parse_opt ( int argc, char* * argv );

int main ( int argc, char* argv[] ) {

    image_t out;
    image_t *img1, *img2;
    config_t cfg = parse_opt ( argc, argv );

    img1 = read_pnm ( cfg.input_file_1 );
    if ( img1 == NULL ) {
        fprintf ( stderr, "error opening image %s.\n", cfg.input_file_1 );
        return RESULT_ERROR;
    }
    if ( img1->info.result != RESULT_OK ) {
        fprintf ( stderr, "error reading image %s.\n", cfg.input_file_1 );
        pixels_free ( img1->pixels );
        free ( img1 );
        return RESULT_ERROR;
    }
    if ( img1->info.maxval > 1 ) {
        fprintf ( stderr, "only binary images supported.\n" );
        pixels_free ( img1->pixels );
        free ( img1 );
        return RESULT_ERROR;
    }

    img2 = read_pnm ( cfg.input_file_2 );
    if ( img2 == NULL ) {
        fprintf ( stderr, "error opening image %s.\n", cfg.input_file_2 );
        return RESULT_ERROR;
    }
    if ( img2->info.result != RESULT_OK ) {
        fprintf ( stderr, "error reading image %s.\n", cfg.input_file_2 );
        pixels_free ( img1->pixels );
        free ( img1 );
        pixels_free ( img2->pixels );
        free ( img2 );
        return RESULT_ERROR;
    }
    if ( img2->info.maxval > 1 ) {
        fprintf ( stderr, "only binary images supported.\n" );
        pixels_free ( img1->pixels );
        free ( img1 );
        pixels_free ( img2->pixels );
        free ( img2 );
        return RESULT_ERROR;
    }

    out.info = img1->info;
    out.pixels = pixels_copy ( &img1->info, img1->pixels );


    const index_t n = img1->info.width;
    const index_t m = img1->info.height;
    const index_t npatches = m * n;
    //
    //
    //
    long a = 0;
    long n1 = 0;
    index_t n00 = 0, n01 = 0, n10 = 0, n11 = 0;
    for ( int i = 0, li = 0 ; i < m ; ++i ) {
        for ( int j = 0 ; j < n ; ++j, ++li ) {
            const int x = get_linear_pixel(img1, li);
            const int y = get_linear_pixel(img2, li);
            if (x == 0 ) {
                if (y == 0) {
                    n00 ++;
                } else {
                    n01 ++;
                }
            } else{
                if (y == 0) {
                    n10 ++;
                } else {
                    n11 ++;
                }
            }
            n1 += x;
            const int d = x^y;
            set_linear_pixel ( &out, li, d );
            a += d;
        }
    }
    const double k = 100.0/(double)(m*n);
    printf("ref.  0    %9lu (%5.2f%%) 1    %9lu (%5.2f%%) total %9lu\n", m*n-n1, (m*n-n1)*k, n1, n1*k,  m*n );
    printf("equal 0->0 %9lu (%5.2f%%) 1->1 %9lu (%5.2f%%) total %9lu (%5.2f%%)\n", n00, n00*k, n11, n11*k, m*n-a, (m*n-a)*k );
    printf("diff. 0->1 %9lu (%5.2f%%) 1->0 %9lu (%5.2f%%) total %9lu (%5.2f%%)\n", n01, n01*k, n10, n10*k, a, a*k );
    int res = write_pnm ( cfg.output_file, &out );
    if ( res != RESULT_OK ) {
        fprintf ( stderr, "error writing image %s.\n", cfg.output_file );
    }


    pixels_free ( img1->pixels );
    free ( img1 );
    pixels_free ( img2->pixels );
    free ( img2 );
    pixels_free ( out.pixels );
    return res;
}



/**
 * These are the options that we can handle through the command line
 */
static struct argp_option options[] = {
    {"verbose",        'v', 0, OPTION_ARG_OPTIONAL, "Produce verbose output", 0 },
    {"quiet",          'q', 0, OPTION_ARG_OPTIONAL, "Don't produce any output", 0 },
    {"output",         'o', "file",    0, "output file", 0 },
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
    "\n*** compare binary images    ***\n";

/**
 * A general description of the input arguments we accept; appears when calling with --help
 */
static char args_doc[] = "<IMAGE_FILE_1> <IMAGE_FILE_2>";

/**
 * argp configuration structure
 */
static struct argp argp = { options, _parse_opt, args_doc, program_doc, 0, 0, 0 };

config_t parse_opt ( int argc, char* * argv ) {

    config_t cfg;
    cfg.input_file_1  = NULL;
    cfg.input_file_2  = NULL;
    cfg.output_file = "difference.pnm";
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

    case ARGP_KEY_ARG:
        switch ( state->arg_num ) {
        case 0:
            cfg->input_file_1 = arg;
            break;
        case 1:
            cfg->input_file_2 = arg;
            break;
        default:
            /** too many arguments! */
            error ( "Too many arguments!.\n" );
            argp_usage ( state );
            break;
        }
        break;
    case ARGP_KEY_END:
        if ( state->arg_num < 2 ) {
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
