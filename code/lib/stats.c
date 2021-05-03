#include "stats.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

static patch_node_t * create_inner_node (); 

static patch_node_t * create_leaf_node (); 

static const patch_node_t * get_patch_node ( const patch_node_t * ptree, const patch_t * ppatch ); 


/*---------------------------------------------------------------------------------------*/

void update_patch_stats ( const patch_t * pctx, const pixel_t z, patch_node_t * ptree ) {
    patch_node_t * pnode = ptree, * nnode = NULL;
    const int k = pctx->k;
    const pixel_t * const cv = pctx->values;
    register int j;
    /* traverse tree, creating nodes if necessary, and update counts */
    for ( j = 0 ; j < k ; ++j ) {
        pnode->occu++;
        const pixel_t cj = cv[ j ];
        assert ( cj < ALPHA );
        nnode = pnode->children[ cj ];
        if ( nnode == NULL ) {
            if ( j < ( k - 1 ) )
                nnode = pnode->children[ cj ] = create_inner_node ();
            else { // is a leaf.
                nnode = pnode->children[ cj ] = create_leaf_node ();
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
                                    const patch_template_t * ptpl,
                                    patch_mapper_t mapper,
                                    patch_node_t * ptree ) {
    register int i, j;
    const int m = pnoisy->info.height;
    const int n = pnoisy->info.width;
    // temporary patches in stack
    pixel_t ctxval[ ptpl->k ];
    patch_t ctx;
    ctx.k = ptpl->k;
    ctx.values = ctxval;
    
    pixel_t mctxval[ ptpl->k ];
    patch_t mctx; 
    mctx.k = ptpl->k;
    mctx.values = mctxval;

    linear_template_t * ltpl = linearize_template ( ptpl, m, n );
    const int alph_size = pnoisy->info.maxval + 1;
    if ( ptree == NULL ) {
        ptree = create_inner_node ( );
    }
    for ( i = 0 ; i <  m ; ++i ) {
        //if (!(i % 500)) printf("%7d/%7d, #ctx=%ld avgcounts=%ld\n",i,m,num_ctx,(i*n+1)/(num_ctx+1));
        for ( j = 0 ; j <  n ; ++j ) {
#if LINEARIZE
            get_linear_patch ( pctximg, &ltpl, i, j, mapper, &ctx );
#else
            if (mapper == NULL) {
                get_patch ( pctximg, ptpl, i, j, &mctx );
            } else {
                get_mapped_patch ( pctximg, ptpl, i, j, mapper, &ctx, &mctx );

            }
#endif
            const int z = get_pixel ( pnoisy, i, j );
            update_patch_stats ( &mctx, z, ptree );
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
        const int nc = ALPHA;
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


static void _summarize_step_1(patch_node_t * pnode, index_t* nleaves, index_t* maxoccu, index_t* maxcount, index_t* totcount) {
    if ( pnode->leaf ) {
        (*nleaves) ++;
        *totcount += pnode->counts;
        if ( pnode->counts > *maxcount) {
            *maxcount = pnode->counts;
        }
        if ( pnode->occu > *maxoccu) {
            *maxoccu = pnode->occu;
        }
    } else {
        int i;
        for ( i = 0 ; i < ALPHA ; ++i ) {
            if ( pnode->children[ i ] )  {
                _summarize_step_1( pnode->children[ i ], nleaves, maxoccu, maxcount, totcount );
            }
        }
    }
}

static void _summarize_step_2(patch_node_t * pnode, index_t* count_counts) {
    if ( pnode->leaf ) {
        count_counts[ pnode->counts ] ++;
    } else {
        int i;
        for ( i = 0 ; i < ALPHA ; ++i ) {
            if ( pnode->children[ i ] )  {
                _summarize_step_2( pnode->children[ i ], count_counts );
            }
        }
    }
}

void summarize_patch_stats ( patch_node_t * pnode, const char* prefix ) {
    index_t nleaves   = 0;
    index_t maxoccu  = 0;
    index_t maxcount  = 0;
    index_t totcount  = 0;
    //
    //
    //
    _summarize_step_1(pnode,&nleaves,&maxoccu,&maxcount,&totcount);
    printf("%s leaves %10ld maxoccu %10ld maxcount %10ld totcount %10ld\n",prefix,nleaves,maxoccu,maxcount,totcount);
    //
    //
    //
    index_t* count_counts = (index_t*)  calloc( (maxcount+1), sizeof(index_t) );
    _summarize_step_2(pnode, count_counts);
    for (index_t i = 0; i < maxcount+1; ++i) {
        if (count_counts[i])
            printf("%s%10ld:%10ld\n",prefix,i,count_counts[i]);
    }
    free(count_counts);
}

/*---------------------------------------------------------------------------------------*/

patch_node_t * create_inner_node () {
    patch_node_t * pnode = ( patch_node_t * ) calloc ( 1, sizeof( patch_node_t ) );
    if ( !pnode ) {
        fprintf ( stderr, "Out of memory." );
    }
    pnode->leaf = 0;
    //memset(pnode->children,0,sizeof(patch_node_t*)*ALPHA);
    return pnode;
}

/*---------------------------------------------------------------------------------------*/

patch_node_t * create_leaf_node ( ) {
    patch_node_t * pnode = ( patch_node_t * ) calloc ( 1, sizeof( patch_node_t ) );
    if ( !pnode ) {
        fprintf ( stderr, "Out of memory." );
    }
    pnode->leaf = 1;    
    return pnode;
}

/*---------------------------------------------------------------------------------------*/
void free_node ( patch_node_t * pnode ) {
    if ( pnode != NULL ) {
        if ( !pnode->leaf ) { // inner node
            if ( pnode->children != NULL ) {
                int i;
                patch_node_t** ch = pnode->children;
                for ( i = 0 ; i < ALPHA ; ++i ) {
                    if (ch[i] != NULL) {
                        free_node ( ch[ i ] );
                        ch[ i ] = NULL;
                    }
                }
                //free ( pnode->children );
                //pnode->children = NULL;
            }
        }
        free ( pnode );
    }
}



/*---------------------------------------------------------------------------------------*/

const patch_node_t * get_patch_node ( const patch_node_t * ptree, const patch_t * pctx ) {
    const patch_node_t * pnode = ptree, * nnode = NULL;
    const int k = pctx->k;
    const pixel_t * const cv = pctx->values;
    register int j;
    for ( j = 0 ; j < k ; ++j ) {
        const pixel_t cj = cv[ j ];
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

index_t get_patch_stats ( const patch_node_t * ptree, const patch_t * pctx ) {
    return get_patch_node ( ptree, pctx )->counts;
}
/*---------------------------------------------------------------------------------------*/

patch_node_t * load_stats ( const char * fname ) {
    return NULL;
}

/*---------------------------------------------------------------------------------------*/

static int write_bit(FILE* fhandle, int bit, unsigned char* pbuffer, unsigned char* pmask) {
    int res;
    if ( *pmask == 0 ) {  // flush the buffer
        if ( ( res = fputc ( *pbuffer, fhandle ) ) == EOF ) {
            return -1;
        }
        *pmask = 0x80;
        *pbuffer = 0x00;
    }
    // write bit
    if ( bit ) {
        *pbuffer |= *pmask;
    }
    // advance buffer
    *pmask >>= 1;
    return 0;
}
static int write_count(FILE* handle, index_t counts) {
    uint64_t counts64 = counts;
    unsigned char* bytes = (unsigned char*) &counts64;
    // sorry, will only work on little-endian machines
    for (int i = 0; i < sizeof(uint64_t); ++i) {
        fputc(bytes[i], handle);
    }
    return 0;
}

int save_node(FILE* handle, const patch_node_t* node, unsigned char* buffer, unsigned char* mask) {
    write_bit(handle,node->leaf,buffer,mask);
    if (node->leaf) {
        write_count(handle,node->occu);
        return write_count(handle,node->counts);
    } else {
        for (int i = 0; i < ALPHA; ++i) {
            if (node->children[i]) {
                write_bit(handle,1,buffer,mask); // present bit
                save_node(handle,node->children[i],buffer,mask);
            } else {
                write_bit(handle,0,buffer,mask); // not present
            }
        }
    }
    return 0;
}

int save_stats ( const char * fname, const patch_node_t * ptree ) {
    //
    // binary encoded
    // the first bit indicates whether the node is inner (0) or leaf (1) 
    // if inner, for each child:
    //   use 1 bit to indicate whether it is present (1) or absent (0)
    //   if present, 
    //     encode it    
    // if leaf, 
    //   encode occurences using 64 bits (pending: use Golomb)
    //   encode counts using 64 bits (pending: use Golomb)
    //
    FILE* handle = fopen(fname,"wb");
    if (!handle) {
        fprintf(stderr,"Error writing stats file %s.",fname);
        return -1;
    }
    unsigned char buffer = 0x00;
    unsigned char mask   = 0x80;
    save_node(handle,ptree,&buffer,&mask);
    // flush byte buffer
    mask = 0;
    write_bit(handle,0,&buffer,&mask);
    fclose(handle);
    return 0;
}
