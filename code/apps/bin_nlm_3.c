/**
 * binarized non-local means
 * the patches are binarized and compared as bit fields
 * the weights are still Gaussian
 * the center values, however, are treated as signed integers
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

upixel_t* all_patches;

upixel_t * extract_patches ( const image_t* img, const patch_template_t* tpl ) {
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
            printf("i %d j %d patch:",i,j);
            for (int kk = 0; kk < ko; kk++) {
                printf("%x ",*(all_patches + li*ko + kk));
            }
            printf("\n");
#endif
        }
    }
    free_linear_template ( ltpl );
    free_patch ( q );
    free_patch ( p );
    return all_patches;
}

/**
 * the distance between two patches is computed as the number of different
 * bits between their binary representations.
 * we must handle raw bytes and conver them to the appropriate sizes
 */
int patch_dist (const index_t i, const index_t j, const patch_template_t* tpl ) {
    const index_t nsamples = compute_binary_mapping_samples ( tpl->k );
    const upixel_t* samplesi = &all_patches[ nsamples * i ];
    const upixel_t* samplesj = &all_patches[ nsamples * j ];
#ifdef INSANE_DEBUG
    printf("dist: patch %d:",i);
    for (int kk = 0; kk < nsamples; kk++) {
        printf("%x ", samplesi[kk] );
    }
    printf("\n");
    printf("dist: patch %d:",j);
    for (int kk = 0; kk < nsamples; kk++) {
        printf("%x ",samplesj[kk] );
    }
    printf("\n");
#endif
    int d = 0;
    for ( index_t k = 0 ; k < nsamples ; ++k ) {
        upixel_t bdif = samplesi[ k ] ^ samplesj[ k ];
        d += block_weight ( bdif );
    }
    return d;
}

int main ( int argc, char* argv[] ) {
    char ofname[ 128 ];
    if ( argc < 2 ) {
        fprintf ( stderr, "usage: %s <image>.\n", argv[ 0 ] );
        return RESULT_ERROR;
    }
    const char* fname = argv[ 1 ];
    image_t* img = read_pnm ( fname );
    if ( img == NULL ) {
        fprintf ( stderr, "error opening image %s.\n", fname );
        return RESULT_ERROR;
    }
    if ( img->info.result != RESULT_OK ) {
        fprintf ( stderr, "error reading image %s.\n", fname );
        pixels_free ( img->pixels );
        free ( img );
        return RESULT_ERROR;
    }
    if ( img->info.maxval > 1) {
        fprintf ( stderr, "only binary images supported.\n" );
        pixels_free ( img->pixels );
        free ( img );
        return RESULT_ERROR;
    }
    image_t out;
    out.info = img->info;
    out.pixels = pixels_copy ( &img->info, img->pixels );
    //
    //
    //
    const int m = img->info.height;
    const int n = img->info.width;
    //
    // create template
    //
    const int radius = 3;
    const int norm = 2;
    const int exclude_center = 1;
    patch_template_t* tpl;

    tpl = generate_ball_template ( radius, norm, exclude_center );
    const index_t maxd = 5;
    index_t w[maxd];
    for (index_t d = 0; d < maxd; ++d) {
        w[d] = 1024/(d+1);
    }
    //
    // non-local means
    // search a window of size R
    //
    printf ( "extracting patches....\n" );
    extract_patches ( img, tpl );

    printf ( "denoising....\n" );
    const int R = 20;
    for ( int i = 0, li = 0 ; i < m ; ++i ) {
        for ( int j = 0 ; j < n ; ++j, ++li ) {
            //const pixel_t z = get_linear_pixel ( img, li );
            double y = 0.0;
            double norm = 0;
            int di0 = i > R     ? i - R : 0;
            int di1 = i < ( m - R ) ? i + R : m;
            int dj0 = j > R     ? j - R : 0;
            int dj1 = j < ( n - R ) ? j + R : n;
            for ( int di = di0 ; di < di1 ; ++di ) {
                for ( int dj = dj0 ; dj < dj1 ; ++dj ) {
                    const index_t lj = di*n + dj;
                    const int d = patch_dist (li, lj, tpl );
                    if (d > maxd) {
                        continue;
                    }
                    y += w[d-1] * (get_pixel ( img, di, dj ));                    
                    norm += w[d-1];
                }
            }
            set_linear_pixel ( &out, li, y/norm < 0.5 ? 0: 1);
        }
	if (!(i % 100)) {
	    printf("row %d\n",i);
	}
    }

    printf ( "saving result...\n" );
    snprintf ( ofname, 128, "nlm_%s", fname );
    // OVERRIDE since writing raw binary type is broken
    out.info.type = 1;
    out.info.encoding = PNM_ASCII;
    int res = write_pnm ( ofname, &out );
    if ( res != RESULT_OK ) {
        fprintf ( stderr, "error writing image %s.\n", ofname );
    }


    printf ( "finishing...\n" );
    free ( all_patches );
    free_patch_template ( tpl );
    pixels_free ( img->pixels );
    pixels_free ( out.pixels );
    free ( img );
    return res;
}