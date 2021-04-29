#include "stats.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

static patch_node_t * create_node ( const int leaf, const int alph_size ); // should be static

static const patch_node_t * get_patch_node ( const patch_node_t * ptree, const patch_t * ppatch ); // should be static


/*---------------------------------------------------------------------------------------*/

void update_patch_stats ( const patch_t * pctx, const val_t z, patch_node_t * ptree ) {
    patch_node_t * pnode = ptree, * nnode = NULL;
    const int k = pctx->k;
    const val_t * const cv = pctx->values;
    register int j;
    /* traverse tree, creating nodes if necessary, and update counts */
    for ( j = 0 ; j < k ; ++j ) {
        pnode->occu++;
        const int ctx_alph = pnode->nchildren;
        const val_t cj = cv[ j ];
        if ( cj >= ctx_alph ) { printf ( "cj %d alph %d\n", cj, ctx_alph ); }
        assert ( cj < ctx_alph );
        nnode = pnode->children[ cj ];
        if ( nnode == NULL ) {
            if ( j < ( k - 1 ) )
                nnode = pnode->children[ cj ] = create_node ( 0, ctx_alph );
            else { // is a leaf.
                nnode = pnode->children[ cj ] = create_node ( 1, ctx_alph );
            }
        }
        pnode = nnode;
    }
    // this one is always a leaf, and the contents of the node are the average
    pnode->occu++;
    pnode->counts += z;
}

/*---------------------------------------------------------------------------------------*/

patch_node_t * gather_patch_stats ( const image_t * pnoisy,
                                    const image_t * pctximg,
                                    const template_t * ptpl,
                                    patch_mapper_t mapper,
                                    patch_node_t * ptree ) {
    register int i, j;
    const int m = pnoisy->info.height;
    const int n = pnoisy->info.width;
    val_t ctxval[ ptpl->k ];
    patch_t ctx;
    ctx.k = ptpl->k;
    linear_template_t * ltpl = linearize_template ( ptpl, m, n );
    ctx.values = ctxval;
    const int alph_size = pnoisy->info.maxval + 1;
    if ( ptree == NULL ) {
        ptree = create_node ( 0, alph_size );
    }
    for ( i = 0 ; i <  m ; ++i ) {
        //if (!(i % 500)) printf("%7d/%7d, #ctx=%ld avgcounts=%ld\n",i,m,num_ctx,(i*n+1)/(num_ctx+1));
        for ( j = 0 ; j <  n ; ++j ) {
#if LINEARIZE
            get_linear_patch ( pctximg, &ltpl, i, j, mapper, &ctx );
#else
            get_patch ( pctximg, ptpl, i, j, mapper, &ctx );
#endif
            const int z = get_pixel ( pnoisy, i, j );
            update_patch_stats ( &ctx, z, ptree );
        }
    }
    free_linear_template ( ltpl );
    return ptree;
}

/*---------------------------------------------------------------------------------------*/

void print_patch_stats ( patch_node_t * pnode, char * prefix ) {
    if ( pnode->leaf ) {
        printf ( "%s %10ld / %10ld\n", prefix, pnode->counts, pnode->occu );
    } else {
        int i;
        const int nc = pnode->nchildren;
        for ( i = 0 ; i < nc ; ++i ) {
            if ( pnode->children[ i ] )  {
                char tmp[ 16 ];
                snprintf ( tmp, 16, "%6d, ", i );
                strncat ( prefix, tmp, 1023 );
                print_patch_stats ( pnode->children[ i ], prefix );
                prefix[ strlen ( prefix ) - strlen ( tmp ) ] = 0;
            }
        }
    }
}

/*---------------------------------------------------------------------------------------*/

patch_node_t * create_node ( const int leaf, const int alph_size ) {
    patch_node_t * pnode = ( patch_node_t * ) calloc ( 1, sizeof( patch_node_t ) );
    pnode->leaf = leaf;
    if ( !leaf ) {
        pnode->children = ( patch_node_t * * ) calloc ( alph_size, sizeof( patch_node_t * ) );
        if ( !pnode->children ) {
            fprintf ( stderr, "Out of memory." );
        }
        pnode->nchildren = alph_size;
    } else {
        pnode->nchildren = 0;
    }
    return pnode;
}

/*---------------------------------------------------------------------------------------*/
void free_node ( patch_node_t * pnode ) {
    if ( pnode != NULL ) {
        if ( !pnode->leaf ) { // inner node
            if ( pnode->children != NULL ) {
                int i;
                patch_node_t * * ch = pnode->children;
                const int nc = pnode->nchildren;
                for ( i = 0 ; i < nc ; ++i ) {
                    free_node ( ch[ i ] );
                    ch[ i ] = NULL;
                }
                free ( pnode->children );
                pnode->children = NULL;
            }
        }
        free ( pnode );
    }
}



/*---------------------------------------------------------------------------------------*/

const patch_node_t * get_patch_node ( const patch_node_t * ptree, const patch_t * pctx ) {
    const patch_node_t * pnode = ptree, * nnode = NULL;
    const int k = pctx->k;
    const val_t * const cv = pctx->values;
    register int j;
    for ( j = 0 ; j < k ; ++j ) {
        const val_t cj = cv[ j ];
        nnode = pnode->children[ cj ];
        if ( nnode == NULL ) {
            fprintf ( stderr, "Error: patch node not found at depth=%d c[j]=%d\n", j, cj );
            return NULL;
        }
        pnode = nnode;
    }
    // this one is always a leaf, and the contents of the node are the average
    return pnode;
}

/*---------------------------------------------------------------------------------------*/

count_t get_patch_stats ( const patch_node_t * ptree, const patch_t * pctx ) {
    return get_patch_node ( ptree, pctx )->counts;
}
/*---------------------------------------------------------------------------------------*/

patch_node_t * load_stats ( const char * fname ) {
    return NULL;
}

/*---------------------------------------------------------------------------------------*/

void save_stats ( const char * fname, const patch_node_t * ptree ) {

}
