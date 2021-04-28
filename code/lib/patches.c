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



