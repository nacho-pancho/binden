/**
 * binarized non-local means
 * the patches are binarized and compared as bit fields
 * the weights are of the form 1/d, with d = hamming distance
 * between the target and the src patch
 * the center values are as in NLM, a simple average of the
 * weighted samples.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h> // for memcmp

#include "pnm.h"
#include "image.h"
#include "templates.h"
#include "patches.h"
#include "patch_mapper.h"
#include "bitfun.h"
#include "config.h"

/*---------------------------------------------------------------------------------------*/

static upixel_t* all_patches;

static upixel_t * extract_patches ( const image_t* img, const patch_template_t* tpl ) {
    //
    // determine total number of patches in image
    //
    const index_t n = img->info.width;
    const index_t m = img->info.height;
    const index_t npatches = m * n;
    const size_t ki = tpl->k;
    const size_t ko = compute_binary_mapping_samples ( ki );
    printf ( "allocating %ld bytes for all %ld patches\n", sizeof( upixel_t ) * ko * npatches, npatches );
    all_patches = ( upixel_t* ) malloc ( ko * npatches * sizeof( upixel_t ) );
    patch_t* p = alloc_patch ( ki );
    patch_t* q = alloc_patch ( ko );
    linear_template_t* ltpl = linearize_template ( tpl, m, n );
    for ( int i = 0, li = 0 ; i < m ; ++i ) {
        for ( int j = 0 ; j < n ; ++j, ++li ) {
            get_linear_patch ( img, ltpl, i, j, p );
            //
            // binarize
            //
            binary_patch_mapper ( p, q );
            // copy raw bytes: this bypasses sign, which is good for us
            memcpy ( all_patches + li * ko, q->values, ko * sizeof( upixel_t ) );
#ifdef INSANE_DEBUG
            printf ( "i %d j %d patch:", i, j );
            for ( int kk = 0 ; kk < ko ; kk++ ) {
                printf ( "%x ", *( all_patches + li * ko + kk ) );
            }
            printf ( "\n" );
#endif
        }
    }
    free_linear_template ( ltpl );
    free_patch ( q );
    free_patch ( p );
    return all_patches;
}

static float * create_gaussian_weights ( const patch_template_t* tpl, const float sigma ) {
    const int k = tpl->k;
    float* weights = ( float* ) malloc ( k * sizeof( float ) );
    float n = 0.0f;
    for ( int r = 0 ; r < k ; ++r ) {
        const float i = fabs ( ( double ) tpl->coords[ r ].i );
        const float j = fabs ( ( double ) tpl->coords[ r ].j );
        const float w = exp ( -0.5 * ( i * i + j * j ) / ( sigma * sigma ) );
        weights[ r ] = w;
        n += w;
    }
    // normalize  so that sum is 1
    for ( int r = 0 ; r < k ; ++r ) {
        weights[ r ] /= n;
    }
    return weights;
}

/*---------------------------------------------------------------------------------------*/

/**
 * the distance between two patches is computed as the number of different
 * bits between their binary representations.
 * we must handle raw bytes and conver them to the appropriate sizes
 */
static int patch_dist ( const index_t i, const index_t j, const patch_template_t* tpl ) {
    const index_t nsamples = compute_binary_mapping_samples ( tpl->k );
    const upixel_t* samplesi = &all_patches[ nsamples * i ];
    const upixel_t* samplesj = &all_patches[ nsamples * j ];
#ifdef INSANE_DEBUG
    printf ( "dist: patch %d:", i );
    for ( int kk = 0 ; kk < nsamples ; kk++ ) {
        printf ( "%x ", samplesi[ kk ] );
    }
    printf ( "\n" );
    printf ( "dist: patch %d:", j );
    for ( int kk = 0 ; kk < nsamples ; kk++ ) {
        printf ( "%x ", samplesj[ kk ] );
    }
    printf ( "\n" );
#endif
    int d = 0;
    for ( index_t k = 0 ; k < nsamples ; ++k ) {
        upixel_t bdif = samplesi[ k ] ^ samplesj[ k ];
        d += block_weight ( bdif );
    }
    return d;
}

/*---------------------------------------------------------------------------------------*/

static index_t apply_denoiser ( image_t* out, const image_t* img,
                         const patch_template_t* tpl, config_t* cfg ) {

    const index_t R = cfg->search_radius;
    const index_t maxd = cfg->max_dist;
    const double perr = cfg->perr;

    const double h = cfg->nlm_weight_scale;
    const double C = -0.5 / ( h * h );
    float* w = create_gaussian_weights ( tpl, h );

    //index_t w[ maxd ];
    //for ( index_t d = 0 ; d < maxd ; ++d ) {
    //    w[ d ] = 1024 / ( ( d >> cfg->decay ) + 1 );
    //    printf ( "dist %ld weight %ld\n", d, w[ d ] );
    //}

    const int m = img->info.height;
    const int n = img->info.width;
    index_t oned = 0;
    index_t zeroed = 0;
    for ( int i = 0, li = 0 ; i < m ; ++i ) {
        for ( int j = 0 ; j < n ; ++j, ++li ) {
            double y = 0.0;
            double norm = 0.0;
            int di0 = i > R     ? i - R : 0;
            int di1 = i < ( m - R ) ? i + R : m;
            int dj0 = j > R     ? j - R : 0;
            int dj1 = j < ( n - R ) ? j + R : n;
            for ( int di = di0 ; di < di1 ; ++di ) {
                for ( int dj = dj0 ; dj < dj1 ; ++dj ) {
                    const index_t lj = di * n + dj;
                    const int d = patch_dist ( li, lj, tpl );
                    if ( d > maxd ) {
                        continue;
                    }
                    if ( get_pixel ( img, di, dj ) ) {
                        y += w[ d - 1 ];
                    }
                    norm += w[ d - 1 ];
                }
            }
            const pixel_t z = get_linear_pixel ( img, li );
            const pixel_t x = cfg->denoiser ( z, y, norm, perr );
            if ( z != x ) {
                set_linear_pixel ( out, li, x );
                if ( x )
                    oned++;
                else
                    zeroed++;
            }
        }
        if ( cfg->verbose && !( i % 500 ) ) {
            printf ( "| %6d | 1->0 %8ld | 0->1 %8ld |\n", i, zeroed, oned );
        }
    }
    free(w);
    return zeroed + oned;
}

/*---------------------------------------------------------------------------------------*/

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
    patch_template_t* tpl;
    if ( !cfg.template_file || !strlen(cfg.template_file)) {
        fprintf ( stderr, "a template is required for this method.\n" );
        pixels_free ( img->pixels );
        free ( img );
        return RESULT_ERROR;
    }
    tpl = read_template ( cfg.template_file );
    if (!tpl) {
        fprintf ( stderr, "missing or invalid template file %s.\n",cfg.template_file );
        pixels_free ( img->pixels );
        free ( img );
        return RESULT_ERROR;
    }
    sort_template ( tpl, 1 );
    //dilate_template ( tpl, cfg.template_scale, 1 );

    image_t out;
    out.info = img->info;
    out.pixels = pixels_copy ( &img->info, img->pixels );
    //
    //
    //
    //
    // create template
    //

    //
    // non-local means
    // search a window of size R
    //
    printf ( "extracting patches....\n" );
    extract_patches ( img, tpl );

    printf ( "denoising / first pass....\n" );
    apply_denoiser ( &out, img, tpl, &cfg );

    printf ( "denoising / second pass....\n" );
    extract_patches ( &out, tpl );
    apply_denoiser ( &out, img, tpl, &cfg );

    printf ( "saving result...\n" );
    int res = write_pnm ( cfg.output_file, &out );
    if ( res != RESULT_OK ) {
        fprintf ( stderr, "error writing image %s.\n", cfg.output_file );
    }

    printf ( "finishing...\n" );
    free ( all_patches );
    free_patch_template ( tpl );
    pixels_free ( img->pixels );
    pixels_free ( out.pixels );
    free ( img );
    return res;
}

/*---------------------------------------------------------------------------------------*/
