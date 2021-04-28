#include "stats.h"
#include <string.h>

/*---------------------------------------------------------------------------------------*/

void update_patch_stats(const patch_t* pctx, const val_t z, stats_updater_t update_stats, patch_node_t* ptree) {
    patch_node_t* pnode = ptree, *nnode = NULL;
    const int k = pctx->k;
    const val_t* const cv = pctx->values;
    register int j;
    /* traverse tree, creating nodes if necessary, and update counts */
    for (j = 0; j < k; ++j) {
        pnode->occu++;
        const int ctx_alph = pnode->nchildren;
        const val_t cj = cv[j];
        nnode = pnode->children[cj];
        if (nnode == NULL) {
            if (j < (k-1))
                nnode = pnode->children[cj] = create_node(0,ctx_alph);
            else {  // is a leaf.
                nnode = pnode->children[cj] = create_node(1,ctx_alph);
            }
        }
        pnode = nnode;
    }
    // this one is always a leaf, and the contents of the node are the average
    pnode->occu++;
    update_stats(&nnode->stats,z);
}

/*---------------------------------------------------------------------------------------*/

patch_node_t* gather_patch_stats(const image_t* pnoisy,
                                  const image_t* pctximg,
                                  const template_t* ptpl,
                                  patch_mapper_t mapper,
                                  stats_updater_t update_stats,
                                  patch_node_t* ptree) {
    register int i,j;
    const int m = pnoisy->info.height;
    const int n = pnoisy->info.width;
    val_t ctxval[ptpl->k];
    patch_t ctx;
    ctx.k = ptpl->k;
    index_t li[ptpl->k];
    linear_template_t ltpl;
    ltpl.k = ptpl->k;
    ltpl.li = li;
    linearize_template(ptpl,m,n,&ltpl);
    ctx.values = ctxval;
    const int alph_size = pnoisy->info.maxval + 1;
    if (ptree == NULL) {
        ptree = create_node(0,alph_size);
    }
    for (i = 0 ; i <  m; ++i) {
        //if (!(i % 500)) printf("%7d/%7d, #ctx=%ld avgcounts=%ld\n",i,m,num_ctx,(i*n+1)/(num_ctx+1));
        for (j = 0 ; j <  n; ++j) {	    
#if LINEARIZE
            get_linear_patch(pctximg,&ltpl,i,j,mapper,&ctx);
#else
            get_patch(pctximg,ptpl,i,j,mapper,&ctx);
#endif
            const int z = get_pixel(pnoisy,i,j);
            update_patch_stats(&ctx,z,update_stats,ptree);
        }
    }
    return ptree;
}

/*---------------------------------------------------------------------------------------*/

void print_patch_stats(patch_node_t* pnode, char* prefix, stats_printer_t print_stats) {
    if (pnode->leaf) {
        printf("%s (%ld)",prefix,pnode->occu);
        print_stats(pnode->stats);
        putchar('\n');
    } else {
        int i;
        const int nc = pnode->nchildren;
        for (i = 0; i < nc; ++i) {
            if (pnode->children[i])  {
                char tmp[16];
                snprintf(tmp,16,"%6d, ",i);
                strncat(prefix,tmp,1023);
                print_patch_stats(pnode->children[i],prefix,print_stats);
                prefix[strlen(prefix)-strlen(tmp)] = 0;
            }
        }
    }
}


