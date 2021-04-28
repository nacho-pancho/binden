//
// standard libraries
//
#include <stdlib.h>
//
// own modules
//
#include "ascmat.h"
#include "patches.h"

#define LINEARIZE 0

/*---------------------------------------------------------------------------------------*/

void get_patch(const image_t* pimg, const template_t* ptpl, int i, int j, patch_mapper_t mapper, patch_t* pctx) {
    const int k = ptpl->k;
    register int r;
    for (r = 0; r < k; ++r) {
        pctx->values[r] = get_pixel(pimg, i+ptpl->is[r], j+ptpl->js[r]);
    }
    if (mapper!=NULL) mapper(pctx,pctx);
}

/*---------------------------------------------------------------------------------------*/

void print_patch(const patch_t* pctx) {
    int j;
    printf("[ ");
    for (j=0; j < pctx->k; j++) {
       printf("%03d ",pctx->values[j]);
    }
    printf("]\n");
}

/*---------------------------------------------------------------------------------------*/

void get_linear_patch(const image_t* pimg, const linear_template_t* ptpl, int i, int j, patch_mapper_t mapper, patch_t* pctx) {
    const int k = ptpl->k;
    const index_t* const lis = ptpl->li;
    const index_t offset = i*pimg->info.width+j;
    val_t* pcv = pctx->values;
    register int r;
    for (r = 0; r < k; ++r) {
        pcv[r] = get_linear_pixel(pimg, offset+lis[r]);
    }
    if (mapper!=NULL) mapper(pctx,pctx);
}


/*---------------------------------------------------------------------------------------*/

patch_node_t* create_node(const int leaf, const int alph_size) {
    patch_node_t* pnode = (patch_node_t*) calloc(sizeof(patch_node_t),1);
    pnode->leaf = leaf;
    if (!leaf) {
        pnode->children = (patch_node_t**) calloc(sizeof(patch_node_t*),alph_size);
        if (!pnode->children) {
            fprintf(stderr,"Out of memory.");
        }
    } else {
        pnode->stats = NULL;
        //num_ctx++; was a global variable counting the total number of contexts created
    }
    return pnode;
}

/*---------------------------------------------------------------------------------------*/
void destroy_node(patch_node_t* pnode) {
    if (pnode != NULL) {
        if (pnode->leaf) { // leaf node
            free(pnode->stats);
        } else { // inner node
            if (pnode->children != NULL) {
                int i;
                patch_node_t** ch = pnode->children;
                const int nc = pnode->nchildren;
                for (i = 0; i < nc; ++i) {
                    destroy_node(ch[i]);
                    ch[i] = NULL;
                }
                free(pnode->children);
                pnode->children = NULL;
            }
        }
        free(pnode);
    }
}



/*---------------------------------------------------------------------------------------*/

const patch_node_t* get_patch_node(const patch_node_t* ptree, const patch_t* pctx) {
    const patch_node_t* pnode = ptree, *nnode = NULL;
    const int k = pctx->k;
    const val_t* const cv = pctx->values;
    register int j;
    for (j = 0; j < k; ++j) {
        const val_t cj = cv[j];
        nnode = pnode->children[cj];
        if (nnode == NULL) {
            fprintf(stderr,"Error: patch node not found at depth=%d c[j]=%d\n",j,cj);
            return NULL;
        }
        pnode = nnode;
    }
    // this one is always a leaf, and the contents of the node are the average
    return pnode;
}

void* get_patch_stats(const patch_node_t* ptree, const patch_t* pctx) {
  return get_patch_node(ptree,pctx)->stats;
}

