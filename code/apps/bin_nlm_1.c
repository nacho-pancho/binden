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

uint8_t* extract_patches(const image_t* img, const patch_template_t* tpl) {
    //
    // determine total number of patches in image
    //
    const index_t n = img->info.width;
    const index_t m = img->info.height;
    const index_t npatches = m*n;
    const size_t sbytes = sizeof(upixel_t);
    const size_t sbits = 8*sbytes;
    const size_t ki = tpl->k;
    const size_t ko = tpl->k / sbits + (tpl->k % sbits ? 1 : 0);
    printf("allocating %ld bytes for all %ld patches\n",ko*npatches,npatches);
    uint8_t* all_patches = malloc(ko*npatches);
    patch_t* p = alloc_patch(ki);
    patch_t* q = alloc_patch(ko);
    linear_template_t* ltpl = linearize_template(tpl,m,n);
    for (int i = 0, li = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j, ++li) {
            get_linear_patch(img,ltpl,i,j,p);
            //
            // remove mean
            //
            int mean = 0;
            for (int r = 0; r < p->k; ++r) 
                mean += p->values[r];
            mean /= p->k;
            for (int r = 0; r < p->k; ++r) 
                p->values[r] -= mean;            
            //
            // binarize
            //
            binary_patch_mapper(p,q);
            // copy raw bytes
            memcpy(all_patches+li*sbytes,q->values,sbytes);
        }
    } 
    free_linear_template(ltpl);
    free_patch(q);
    free_patch(p);
    return all_patches;
}

/**
 * the distance between two patches is computed as the number of different
 * bits between their binary representations.
 * we must handle raw bytes and conver them to the appropriate sizes 
 */
int patch_dist(uint8_t* all_patches, const index_t i, const index_t j, const size_t nbytes) {
    const uint8_t* bytesi = all_patches[nbytes*i];
    const uint8_t* bytesj = all_patches[nbytes*j];
    int d = 0;
    for (int k = 0; k < nbytes; ++k) {
        
    }
}

int main(int argc, char* argv[]) {
    char ofname[128];
    if (argc < 2) { 
        fprintf(stderr,"usage: %s <image>.\n",argv[0]); 
        return RESULT_ERROR; 
    }
    const char* fname = argv[1];    
    image_t* img = read_pnm(fname);
    if (img == NULL) {
        fprintf(stderr,"error opening image %s.\n",fname);
        return RESULT_ERROR;
    }
    if (img->info.result != RESULT_OK) {
        fprintf(stderr,"error reading image %s.\n",fname);
        pixels_free(img->pixels);
        free(img);
        return RESULT_ERROR;
    }
    image_t out;
    out.info = img->info;
    out.pixels = pixels_copy(&img->info,img->pixels);
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
    const int exclude_center = 0;
    patch_template_t* tpl;
    
    tpl = generate_ball_template(radius,norm,exclude_center);
    float sigma = radius;
    //
    // non-local means
    // search a window of size R
    //
    printf("extracting patches....\n");
    uint8_t* all_patches = extract_patches(img,tpl);
    patch_dist(all_patches,100,200);

    printf("denoising....\n");
    for (int i = 0, li = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j, ++li) {
            const pixel_t z = get_linear_pixel(img,li);
            const pixel_t x = z;
            set_linear_pixel(&out,li,x);
        }
    }

    printf("saving result...\n");
    snprintf(ofname,128,"nlm_%s",fname);
    int res = write_pnm(ofname,&out);
    if (res != RESULT_OK) {
        fprintf(stderr,"error writing image %s.\n",ofname);
    }


    printf("finishing...\n");
    free(all_patches);
    free_patch_template(tpl);
    pixels_free(img->pixels);
    pixels_free(out.pixels);
    free(img);
    return res;
}
