#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "stats.h"
#include "logging.h"


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
        //
        // delete subtree
        //
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
        free(pnode);
    }
}

/*---------------------------------------------------------------------------------------*/

void delete_node ( patch_node_t * node) {
    patch_node_t* parent = node->parent;
    pixel_t val = node->value;
    free(node); 
    if (parent != NULL) {
        parent->children[val] = NULL; // remove from parent
        for (int i = 0; i < ALPHA; ++i) {
            if (parent->children[i] != NULL) {
                return; // don't kill me! I have kids!
            }
        }
        // say goodbye!
        delete_node(parent);
    }
}

/*---------------------------------------------------------------------------------------*/

void merge_nodes ( patch_node_t * dest, patch_node_t* src) {
    dest->occu += src->occu;
    dest->counts += src->counts;
    delete_node(src);
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

/*---------------------------------------------------------------------------------------*/

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
}


/**
 * return a list of all the patches that are a given distance to the
 * specified center 
 */
static void find_neighbors_inner ( 
    neighbor_list_t* nlist, 
    const index_t dist, 
    patch_node_t* ptree, 
    const patch_t* center, 
    const index_t patch_pos,   
    const index_t maxd) {
    //printf("find_neighbors_inner pos %ld size %d ctr %d dist %ld maxd %ld neigh %ld maxn %ld\n",
    //    patch_pos, center->k, center->values[patch_pos], dist, maxd, nlist->number, maxn);
    if (ptree->leaf) {
      // do not inclde center itself
      if (dist == 0) { return; }
        //
        // we arrived at a neighbor
        //
        nlist->neighbors[nlist->number].patch_node = ptree;
        nlist->neighbors[nlist->number++].dist = dist;
        //
        // enlarge list
        //
        if (nlist->number >= nlist->maxnumber) {
            nlist->maxnumber *= 2;
            nlist->neighbors = (neighbor_t*) realloc(nlist->neighbors,nlist->maxnumber*sizeof(neighbor_t));
        }
    } else {
        //
        // inner node: see if we've got budget to go
        //
        for (int i = 0; i < ALPHA; ++i) {
            // next node's value coincides with next position
            // so distance is not increased
            if (!ptree->children[i]) {
                continue;
            }
            const pixel_t val = center->values[patch_pos];
            const index_t downdist = (i == val) ? dist: (dist+1);
            if (downdist > maxd) 
                continue;
            find_neighbors_inner(nlist, downdist, ptree->children[i], center, patch_pos+1, maxd);
        }
    }
    // note: we could speed this up by adding a whole subtree if its depth is
    // lower than the max distance.
}

neighbor_list_t find_neighbors ( 
    patch_node_t* ptree, 
    const patch_t* center, 
    const index_t maxd) {

    neighbor_list_t neighbors;
    neighbors.number = 0;
    neighbors.maxnumber = 1024; // starting size
    neighbors.neighbors = (neighbor_t*) calloc(neighbors.maxnumber,sizeof(neighbor_t));
    const index_t ini_dist = 0;
    const index_t ini_pos  = 0;
    find_neighbors_inner (&neighbors, ini_dist, ptree,  center, ini_pos, maxd);
    return neighbors;
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

patch_node_t * get_patch_node ( patch_node_t * ptree, patch_t * pctx ) {
    patch_node_t * pnode = ptree;
    const int k = pctx->k;
    const pixel_t * const cv = pctx->values;
    register int j;
    for ( j = 0 ; j < k ; ++j ) {
        const pixel_t cj = cv[ j ];
        pnode = pnode->children[ cj ];
        if ( pnode == NULL ) {
            fprintf ( stderr, "Error: patch node not found at depth=%d c[j]=%d\n", j, cj );
            return NULL;
        }
    }
    return pnode;
}

const patch_node_t * get_patch_node_const ( const patch_node_t * ptree, const patch_t * pctx ) {
    return get_patch_node ((patch_node_t*) ptree, (patch_t*) pctx);
}

/*---------------------------------------------------------------------------------------*/

index_t get_patch_stats ( const patch_node_t * ptree, const patch_t * pctx ) {
    return get_patch_node_const ( ptree, pctx )->counts;
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
    //printf("merge stats\n");
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
    //printf("merge nodes\n");
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

/*---------------------------------------------------------------------------------------*/

static int compare_nodes_occu(const void* va, const void* vb) {
    const patch_node_t* a = *((const patch_node_t**) va);
    const patch_node_t* b = *((const patch_node_t**) vb);
    const long res = (b->occu - a->occu); // sort in descending order
    return res > 0 ? 1: (res < 0 ? -1: 0);
}


void sort_stats(patch_node_t** node_list, index_t nnodes) {
    qsort(node_list,nnodes,sizeof(patch_node_t*),compare_nodes_occu);
}

/*---------------------------------------------------------------------------------------*/

void flatten_stats(patch_node_t* node, patch_node_t** node_list, index_t* pos) {
    if (node->leaf) {
        node_list[(*pos)++] = node;
    } else {
        for (int i = 0; i < ALPHA; ++i)
            flatten_stats(node->children[i],node_list,pos);
    }
}

/*---------------------------------------------------------------------------------------*/

static void print_node_list(patch_node_t** node_list, const index_t nnodes, const index_t totoccu, patch_t* aux ) {
    index_t accu = 0;
    for (int i = 0; i < nnodes; ++i) {
        printf("%06d | ",i);
        get_leaf_patch(aux,node_list[i]);
        for (int j = 0; j < aux->k; ++j) {
            putchar('0'+aux->values[j]);
        }
        const patch_node_t* p = node_list[i];
        const double P = ((double) p->counts) / ((double) p->occu);
        const double Q = ((double) p->occu) / ((double) totoccu);
        accu += p->occu;
        const double F = ((double) accu) / ((double) totoccu);
        printf(" | %12ld | %12.10f | %12.10f | %12ld | %12.10f\n", p->occu, Q, F, p->counts, P);
    }
}

/*---------------------------------------------------------------------------------------*/

patch_node_t * clone_stats ( patch_node_t* src ) {
        patch_node_t* out;
        out = alloc_node();
        merge_stats(out,src,1);
        return out;
}

/*---------------------------------------------------------------------------------------*/

static void find_most_popular_cand(patch_node_t * pnode, patch_node_t** currnode, index_t* currmax) {
    //
    // we look for leaf nodes which are not yet assigned to a cluster
    // these are the ones that do not have a probabily vector associated to them
    //
    if ( pnode->leaf && (!pnode->diff) && (pnode->occu > *currmax)) {
            *currmax = pnode->occu;
            *currnode = pnode;
    } else {
        for ( int i = 0 ; i < ALPHA ; ++i ) {
            if ( pnode->children[ i ] )  {
                find_most_popular_cand( pnode->children[ i ], currnode, currmax );
            }
        }
    }
}

patch_node_t * cluster_stats ( patch_node_t* in, const index_t K, const index_t maxd) {
    // working copy; nodes get removed from it
    patch_node_t* work = clone_stats(in);
    // clusters are saved here
    patch_node_t* clusters = create_node(NULL,0,0);
    index_t nleaves =0, noccu =0, ncounts = 0; 
    summarize_stats(work,&nleaves,&noccu,&ncounts);
    patch_t* point = alloc_patch(K);
    patch_t* center = alloc_patch(K);
    index_t maxoccu = 0;
    //
    // we identify as clusters all those patches
    // which are significantly above the expected number 
    // of them arising purely by chance
    // for uniform noise this is simply total_counts/2^{patch size}
    const index_t thres = noccu >> K;
    printf("threshold %ld\n",thres);
    return work;
}
