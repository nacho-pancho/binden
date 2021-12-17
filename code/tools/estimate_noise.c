/**
 * estimates binary asymmetric channel noise parameters P(0->1) and P(1->0)
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h> // for memcmp
#include <argp.h>

#include "pnm.h"
#include "image.h"
#include "templates.h"
#include "patches.h"
#include "logging.h"


/**
 * frequency of each quorum value (size k + 1)
 * P(Q=q)
 */
static index_t* quorum_freq;

/**
 * frequency of each quorum value given that the center is 1
 * each quorum value. (size k + 1)
 */
static index_t* quorum_freq_1;

/**
 * Program options. These are filled in by the argument parser
 */
typedef struct config {
    const char * input_file;
    int template_radius;
    int template_norm;
    int maxs;
    double tol;
} config_t;

config_t parse_opt ( int argc, char* * argv );


static index_t patch_sums ( 
    const image_t* img, 
    const image_t* ctximg, 
    const patch_template_t* tpl,
    index_t* quorum_freq, 
    index_t* quorum_freq_1 ) {
    //
    // determine total number of patches in image
    //
    if ( ctximg == NULL ) {
        ctximg = img;
    }
    const index_t n = img->info.width;
    const index_t m = img->info.height;
    index_t total = 0;
    patch_t* p = alloc_patch ( tpl->k );
    linear_template_t* ltpl = linearize_template ( tpl, m, n );
    for ( int i = 0, li = 0 ; i < m ; ++i ) {
        for ( int j = 0 ; j < n ; ++j, ++li ) {
            get_linear_patch ( ctximg, ltpl, i, j, p );
            //
            // sum
            //
            index_t a = 0;
            for ( int k = 0 ; k < ltpl->k ; ++k ) {
                a += p->values[ k ];
            }
            quorum_freq[ a ]++;
            if ( get_linear_pixel ( img, li ) ) {
                quorum_freq_1[ a ]++;
            }
            total++;
        }
    }
    free_linear_template ( ltpl );
    free_patch ( p );
    return total;
}

/**
 * @brief approximate log-likelihood of all-zeros patches w.r.t. noise parameter
 *        assuming ones are noise
 * @param ns   number of occurences of S=s
 * @param ns1  number of occurences of S=s,z=1
 * @param n    total length of signal
 * @param k    context size
 * @param maxs maximum number of terms
 * @param p0   noise parameter
 * @return double likelihood
 */
static double loglik0(
        const index_t* ns, 
        const index_t* ns1, 
        const index_t n, 
        const index_t k, 
        const index_t maxs, 
        const double p0) {
    const double logp = log10(p0);
    const double log1mp = log10(1.0-p0);
    double a = 0.0;
    for ( int r = 0 ; r <= maxs  ; ++r ) {
      a += ns[r]*(ns1[r]*logp + (ns[r]-ns1[r])*log1mp + r*logp + (k-r)*log1mp);
    }
    return -a/(double)n; // just a normalization
}

/**
 * @brief approximate log-likelihood of all-ONES patches w.r.t. noise parameter
 *        assuming ZEROS are noise
 * @param ns   number of occurences of S=s
 * @param ns1  number of occurences of S=s,z=1
 * @param n    total length of signal
 * @param k    context size
 * @param maxs maximum number of terms
 * @param p0   noise parameter
 * @return double likelihood
 */
static double loglik1(
        const index_t* ns, 
        const index_t* ns1, 
        const index_t n, 
        const index_t k, 
        const index_t maxs, 
        const double p0) {
    const double logp = log10(p0);
    const double log1mp = log10(1.0-p0);
    double a = 0.0;
    for ( int r = k ; r >= (k-maxs)  ; --r ) {
      a += ns[r]*((ns[r]-ns1[r])*logp + ns1[r]*log1mp + (k-r)*logp + r*log1mp);
    }
    return -a/(double)n; // just a normalization
}


static void estimate_noise (
    const image_t* in,
    const index_t k,
    const index_t* quorum_freq,
    const index_t* quorum_freq_1,
    const config_t* cfg ) {

    const int m = in->info.height;
    const int n = in->info.width;
    // compute p1 and p0 using maximum likelihood on the pairs of statistics (n_s, n_s1), s=0,...,k
    // we cannot do this for the whole range s=0,...,k, otherwise we end up with a global estimate
    // which is not what we want. WE only want to evaluate this on "very white" and "very black"
    // contexts. So, assuming a maximum value of 0.2 (quite high), we can discard all terms above
    // s=4 or so.    
    //
    // golden ratio search:
    //
    const double phi = (1.0+sqrt(5.0))/2.0;
    const double r = 1.0/(1.0+phi);
    const double tol = cfg->tol;
    const double maxs = cfg->maxs;

    double left   = 0.0;
    double right  = 0.5;
    while ( (right-left) >= tol) {
        double midleft    = left  + r*(right-left);
        double midright   = right - r*(right-left); 
        const double fmidleft   = loglik0(quorum_freq, quorum_freq_1, m*n, k, maxs, midleft);
        const double fmidright  = loglik0(quorum_freq, quorum_freq_1, m*n, k, maxs, midright);
        if (fmidleft < fmidright) {
            right    = midright;
            midright = midleft;
            midleft  = left  + r*(right-left);
            debug ( "left=%f right=%f p=%8.6f -log P()=%12.10f\n", left,right,midleft,fmidleft); 
        } else {
            left = midleft;
            midleft  = midright;
            midright = right - r*(right-left);
            debug ( "left=%f right=%f p=%8.6f -log P()=%12.10f\n", left,right, midright,fmidright); 
        }
    }
    info("P(0->1)=%f\n",(right+left)/2.0);

    left   = 0.0;
    right  = 0.5;
    while ( (right-left) >= tol) {
        double midleft    = left  + r*(right-left);
        double midright   = right - r*(right-left); 
        const double fmidleft   = loglik1(quorum_freq, quorum_freq_1, m*n, k, maxs, midleft);
        const double fmidright  = loglik1(quorum_freq, quorum_freq_1, m*n, k, maxs, midright);
        if (fmidleft < fmidright) {
            right    = midright;
            midright = midleft;
            midleft  = left  + r*(right-left);
            debug ( "left=%f right=%f p=%8.6f -log P()=%12.10f\n", left,right,midleft,fmidleft); 
        } else {
            left = midleft;
            midleft  = midright;
            midright = right - r*(right-left);
            debug ( "left=%f right=%f p=%8.6f -log P()=%12.10f\n", left,right, midright,fmidright); 
        }
    }
    info("P(1->0)=%f\n",(right+left)/2.0);
}


int main ( int argc, char* argv[] ) {
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
    const int exclude_center = 1;
    patch_template_t* tpl = generate_ball_template(cfg.template_radius,cfg.template_norm,exclude_center);

    quorum_freq   = ( index_t* ) calloc ( tpl->k + 1,  sizeof( index_t ) );
    quorum_freq_1 = ( index_t* ) calloc ( tpl->k + 1,  sizeof( index_t ) );
    //
    // 
    //
    patch_sums ( img, img, tpl, quorum_freq, quorum_freq_1 );
    estimate_noise ( img, tpl->k, quorum_freq, quorum_freq_1, &cfg );
    free ( quorum_freq_1 );
    free ( quorum_freq );
    free_patch_template ( tpl );
    pixels_free ( img->pixels );
    free ( img );
}

/**
 * These are the options that we can handle through the command line
 */
static struct argp_option options[] = {
    {"verbose",        'v', 0, OPTION_ARG_OPTIONAL, "Produce verbose output", 0 },
    {"quiet",          'q', 0, OPTION_ARG_OPTIONAL, "Don't produce any output", 0 },
    {"radius",         'r', "natural", 0, "patch radius", 0 },
    {"norm",           'n', "natural", 0, "patch norm", 0 },
    {"tol",            't', "tolerance", 0, "golden ratio search tolerance", 0 },
    {"maxs",           's', "terms", 0, "number of terms in approx. likelihood", 0 },
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
    cfg.template_radius = 3;
    cfg.template_norm = 2;
    cfg.tol = 1e-6;
    cfg.maxs = 5;
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
    case 'r':
        cfg->template_radius = atoi(arg);
        break;
    case 'n':
        cfg->template_norm = atoi(arg);
        break;
    case 't':
        cfg->tol = atof(arg);
        break;
    case 's':
        cfg->maxs = atoi(arg);
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
