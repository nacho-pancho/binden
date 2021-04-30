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
    p->values = ( pixel_t * ) malloc ( sizeof( pixel_t ) * k );
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
    if (pmapped == NULL) {
        pmapped = pctx; // DESTRUCTIVE
    }        
    get_patch(pimg,ptpl,i,j,pctx);
    mapper( pctx, pmapped );
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
    if (pmapped == NULL) {
        pmapped = pctx; // DESTRUCTIVE
    }        
    get_linear_patch(pimg,ptpl,i,j,pctx);    
    mapper( pctx, pmapped );
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



