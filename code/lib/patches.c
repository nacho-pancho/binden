//
// standard libraries
//
#include <stdlib.h>
//
// own modules
//
#include "ascmat.h"
#include "patches.h"

/*---------------------------------------------------------------------------------------*/

patch_t * alloc_patch ( int k ) {
    patch_t * p = ( patch_t * ) malloc ( sizeof( patch_t ) );
    p->values = ( pixel_t * ) calloc ( k, sizeof( pixel_t ) );
    p->k = k;
    return p;
}

/*---------------------------------------------------------------------------------------*/

void free_patch ( patch_t * p ) {
    free ( p->values );
    free ( p );
}

/*---------------------------------------------------------------------------------------*/

void get_patch ( const image_t * pimg, const patch_template_t * ptpl, int i, int j, patch_t * pctx ) {
    const int k = ptpl->k;
    register int r;
    for ( r = 0 ; r < k ; ++r ) {
        pctx->values[ r ] = get_pixel ( pimg, i + ptpl->coords[ r ].i, j + ptpl->coords[ r ].j );
    }
}

/*---------------------------------------------------------------------------------------*/

void get_mapped_patch ( const image_t * pimg, const patch_template_t * ptpl,
                        int i, int j, patch_mapper_t mapper, patch_t * pctx, patch_t* pmapped ) {
    if ( pmapped == NULL ) {
        pmapped = pctx; // DESTRUCTIVE
    }
    get_patch ( pimg, ptpl, i, j, pctx );
    mapper ( pctx, pmapped );
}

/*---------------------------------------------------------------------------------------*/

void get_linear_patch ( const image_t * pimg, const linear_template_t * ptpl, int i, int j, patch_t * pctx ) {
    const int k = ptpl->k;
    const index_t * const lis = ptpl->li;
    const index_t offset = i * pimg->info.width + j;
    pixel_t * pcv = pctx->values;
    register int r;
    for ( r = 0 ; r < k ; ++r ) {
        pcv[ r ] = get_linear_pixel ( pimg, offset + lis[ r ] );
    }
}

/*---------------------------------------------------------------------------------------*/

void get_mapped_linear_patch ( const image_t * pimg, const linear_template_t * ptpl, int i, int j,
                               patch_mapper_t mapper, patch_t * pctx, patch_t* pmapped ) {
    if ( pmapped == NULL ) {
        pmapped = pctx; // DESTRUCTIVE
    }
    get_linear_patch ( pimg, ptpl, i, j, pctx );
    mapper ( pctx, pmapped );
}


/*---------------------------------------------------------------------------------------*/

void print_patch ( const patch_t * pctx ) {
    int j;
    printf ( "[ " );
    for ( j = 0 ; j < pctx->k ; j++ ) {
        printf ( "%03d ", pctx->values[ j ] );
    }
    printf ( "]\n" );
}


void print_binary_patch ( const patch_t * pctx ) {
    int j;
    for ( j = 0 ; j < pctx->k ; j++ ) {
        fputc ( pctx->values[ j ] ? '1' : '0', stdout );
    }
}


/*---------------------------------------------------------------------------------------*/

void print_patch_fancy ( const patch_t* patch, const patch_template_t * ptpl ) {
    index_t min_i = 10000, min_j = 10000, max_i = -10000, max_j = -10000;
    index_t k, r, l;
    for ( k = 0 ; k < ptpl->k ; k++ ) {
        if ( ptpl->coords[ k ].i < min_i ) min_i = ptpl->coords[ k ].i;
        if ( ptpl->coords[ k ].i > max_i ) max_i = ptpl->coords[ k ].i;
        if ( ptpl->coords[ k ].j < min_j ) min_j = ptpl->coords[ k ].j;
        if ( ptpl->coords[ k ].j > max_j ) max_j = ptpl->coords[ k ].j;
    }
    index_t a = max_i - min_i;
    index_t b = max_j - min_j;
    printf ( "     " );
    for ( r = 0 ; r <= b ; r++ ) {
        printf ( " %3ld ", r + min_j );
    }
    printf ( "\n    +" );
    for ( r = 0 ; r <= b ; r++ ) {
        printf ( "-----" );
    }
    putchar ( '+' );
    putchar ( '\n' );
    for ( k = 0 ; k <= a ; k++ ) {
        printf ( "%3ld |", min_i + k );
        for ( r = 0 ; r <= b ; r++ ) {
            for ( l = 0 ; l < ptpl->k ; l++ ) {
                if ( ( ptpl->coords[ l ].i == ( min_i + k ) ) && ( ptpl->coords[ l ].j == ( min_j + r ) ) ) {
                    printf ( "%4d ", patch->values[ l ] );
                    break;
                }
            }
            if ( l == ptpl->k )
                printf ( "     " );
        }
        putchar ( '|' );
        putchar ( '\n' );
    }
    printf ( "    +" );
    for ( r = 0 ; r <= b ; r++ ) {
        printf ( "-----" );
    }
    putchar ( '+' );
    putchar ( '\n' );
    printf ( "     " );
    for ( r = 0 ; r <= b ; r++ ) {
        printf ( " %3ld ", r + min_j );
    }
    putchar ( '\n' );

}


void print_binary_patch_fancy ( const patch_t* patch, const patch_template_t * ptpl ) {
    index_t min_i = 10000, min_j = 10000, max_i = -10000, max_j = -10000;
    index_t k, r, l;
    for ( k = 0 ; k < ptpl->k ; k++ ) {
        if ( ptpl->coords[ k ].i < min_i ) min_i = ptpl->coords[ k ].i;
        if ( ptpl->coords[ k ].i > max_i ) max_i = ptpl->coords[ k ].i;
        if ( ptpl->coords[ k ].j < min_j ) min_j = ptpl->coords[ k ].j;
        if ( ptpl->coords[ k ].j > max_j ) max_j = ptpl->coords[ k ].j;
    }
    index_t a = max_i - min_i;
    index_t b = max_j - min_j;
    printf ( "+" );
    for ( r = 0 ; r <= b ; r++ ) {
        putchar ( ( r + min_j ) % 5 ? '-' : '+' );
    }
    putchar ( '\n' );
    for ( k = 0 ; k <= a ; k++ ) {
        putchar ( ( min_i + k ) % 5 ? '|' : '+' );
        for ( r = 0 ; r <= b ; r++ ) {
            for ( l = 0 ; l < ptpl->k ; l++ ) {
                if ( ( ptpl->coords[ l ].i == ( min_i + k ) ) && ( ptpl->coords[ l ].j == ( min_j + r ) ) ) {
                    putchar ( patch->values[ l ] ? '1' : '0' );
                    break;
                }
            }
            if ( l == ptpl->k )
                putchar ( ' ' );
        }
        putchar ( '\n' );
    }

}


