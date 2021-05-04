#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "stats.h"
#include "logging.h"

static const patch_node_t * get_patch_node ( const patch_node_t * ptree, const patch_t * ppatch ); 

/*---------------------------------------------------------------------------------------*/
static patch_node_t * alloc_node ( ) {
    patch_node_t* pnode =  ( patch_node_t * ) calloc ( 1, sizeof( patch_node_t ) );
    if ( !pnode ) {
        fprintf ( stderr, "Out of memory." );
    }
    return pnode;
}

/*---------------------------------------------------------------------------------------*/
static patch_node_t * create_node ( patch_node_t* parent, const pixel_t val, char is_leaf ) {
    patch_node_t * pnode = alloc_node();
    pnode->parent = parent;
    pnode->value = val;
    pnode->leaf = is_leaf;
    return pnode;
}

/*---------------------------------------------------------------------------------------*/

static patch_node_t * create_inner_node ( patch_node_t* parent, const pixel_t val ) {
    return create_node(parent,val,0);
}

/*---------------------------------------------------------------------------------------*/

static patch_node_t * create_leaf_node ( patch_node_t* parent, const pixel_t val ) {
    return create_node(parent,val,1);
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

void free_stats ( patch_node_t * pnode ) {
    free_node(pnode);
}

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
                nnode = pnode->children[ cj ] = create_inner_node (pnode, cj);
            else { // is a leaf.
                nnode = pnode->children[ cj ] = create_leaf_node (pnode, cj);
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
    if ( ptree == NULL ) {
        ptree = alloc_node ( );
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


void summarize_stats(patch_node_t * pnode, index_t* nleaves,  index_t* totoccu, index_t* totcount) {
    if ( pnode->leaf ) {
        (*nleaves) ++;
        *totcount += pnode->counts;
        *totoccu += pnode->occu;
    } else {
        int i;
        for ( i = 0 ; i < ALPHA ; ++i ) {
            if ( pnode->children[ i ] )  {
                summarize_stats( pnode->children[ i ], nleaves, totoccu, totcount );
            }
        }
    }
}
#if 0
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
#endif

void print_stats_summary ( patch_node_t * pnode, const char* prefix ) {
    index_t nleaves   = 0;
    index_t totoccu  = 0;
    index_t totcount  = 0;
    //
    //
    //
    summarize_stats(pnode,&nleaves,&totoccu,&totcount);
    printf("%s leaves %10ld totoccu %10ld totcount %10ld\n",prefix,nleaves,totoccu,totcount);
    //
    //
    //
#if 0
    index_t* count_counts = (index_t*)  calloc( (maxcount+1), sizeof(index_t) );
    _summarize_step_2(pnode, count_counts);
    for (index_t i = 0; i < maxcount+1; ++i) {
        if (count_counts[i])
            printf("%s%10ld:%10ld\n",prefix,i,count_counts[i]);
    }
    free(count_counts);
#endif
}

/*---------------------------------------------------------------------------------------*/

void get_leaf_patch ( patch_t * pctx, const patch_node_t * leaf ) {
    assert(leaf->leaf); // must be a leaf
    const patch_node_t* node = leaf;
    for (int j = pctx->k-1; j >= 0; --j) {
        //printf("j %d\n",j);
        assert(node != NULL);
        pctx->values[j] = node->value;
        node = node->parent;
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

#if 0

static int read_bit(FILE* fhandle, unsigned char* pbuffer, unsigned char* pmask) {
    int res;
    unsigned char val;
    if ( *pmask == 0 ) {
        res = fread ( pbuffer, 1, 1, fhandle );
        if ( res != 1 ) return -1;
        *pmask = 0x80;
    }
    val = ( *pbuffer & *pmask ) ? 1 : 0;
    *pmask >>= 1; // shift one bit to right
    return val;
}


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
#endif

static index_t read_count(FILE* fhandle) {
    index_t counts;
    // sorry, will only work on little-endian machines
    size_t res = fread(&counts,sizeof(unsigned char),sizeof(uint64_t), fhandle);
    if (res <  sizeof(uint64_t)) {
        error("error reading stats!");
        return 0;
    }
    return counts;
}

static int write_count(FILE* handle, index_t counts) {
    uint64_t counts64 = counts;
    fwrite(&counts64,sizeof(uint64_t),1,handle);
    return 0;
}

static int read_bool(FILE* fhandle) {
    return fgetc(fhandle) > 0 ? 1: 0;
}

static int write_bool(FILE* handle, int b) {
    unsigned char c = b ? 1 : 0;
    fputc(c,handle);
    return 0;
}

/*---------------------------------------------------------------------------------------*/

int save_node(FILE* handle, const patch_node_t* node) {
    write_bool(handle,node->leaf);
    if (node->leaf) {
        write_count(handle,node->occu);
        return write_count(handle,node->counts);
    } else {
        for (int i = 0; i < ALPHA; ++i) {
            if (node->children[i]) {
                write_bool(handle,1); // present bit
                save_node(handle,node->children[i]);
            } else {
                write_bool(handle,0); // not present
            }
        }
    }
    return 0;
}

/*---------------------------------------------------------------------------------------*/

int read_node(FILE* handle, patch_node_t* node) {
    node->leaf = read_bool(handle);
    if (node->leaf) {
        node->occu   = read_count(handle);
        node->counts = read_count(handle);
    } else {
        for (int i = 0; i < ALPHA; ++i) {
            int has_child = read_bool(handle);
            if (has_child) {
                node->children[i] = create_inner_node( node, i );
                read_node(handle,node->children[i]);
            } else {
                node->children[i] = NULL;
            }
        }
    }
    return 0;
}

/*---------------------------------------------------------------------------------------*/

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
    save_node(handle,ptree);
    fclose(handle);
    return 0;
}

/*---------------------------------------------------------------------------------------*/

patch_node_t * load_stats ( const char * fname ) {
    FILE* handle = fopen(fname,"rb");
    if (!handle) {
        fprintf(stderr,"Error opening stats file %s.",fname);
        return NULL;
    }
    patch_node_t* ptree = alloc_node();
    read_node(handle,ptree);
    fclose(handle);
    return ptree;
}

/*---------------------------------------------------------------------------------------*/

patch_node_t * merge_stats ( patch_node_t* dest, const patch_node_t * src,
    const int in_place ) {
    printf("merge stats\n");
    if ( !in_place ) {
        patch_node_t* out;
        out = alloc_node();
        merge_stats(out,src,1);
        merge_stats(out,dest,1);
        return out;
    }
    //
    // merge node
    //
    printf("merge nodes\n");
    dest->leaf    = src->leaf;
    dest->occu   += src->occu;
    dest->counts += src->counts;
    dest->value   = src->value;
    if (!src->leaf) {
        // recursive merge
        for (int i = 0; i < ALPHA; ++i) {
            // children does not exist in src
            if (src->children[i] == NULL) 
                continue;
            // node exists in src but not in dest: create
            if (dest->children[i] == NULL) {
                dest->children[i] = create_node(dest,i,0);
            }
            merge_stats(dest->children[i],src->children[i],1); // in place, of course
        }
    }
    return dest;
}

