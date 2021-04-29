#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "pnm.h"
#include "image.h"
#include "templates.h"
#include "patches.h"

float* create_gaussian_weights(template_t* tpl, const float sigma) {
    const int k = tpl->k;
    float* weights = (float*) malloc(k*sizeof(float));
    float n = 0.0f;
    for (int r = 0; r < k; ++r) {
        const float i = fabs(tpl->coords[r].i);
        const float j = fabs(tpl->coords[r].j);
        const float w = exp(-0.5*(i*i + j*j)/(sigma*sigma));
	weights[r] = w;
	n += w;
    }
    // normalize  so that sum is 1
    for (int r = 0; r < k; ++r) {
        weights[r] /= n;
    }
    return weights;
}

double patch_dist(const patch_t* a, const patch_t* b, const float* weights) {
    const int k = a->k;
    double dist = 0;
    const val_t* pa = a->values;
    const val_t* pb = b->values;
    for (int r = 0; r < k; ++r) {
        dist += weights[r]*fabs(pa[r]-pb[r]);
    }
    return dist;
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
    template_t* tpl;
    patch_t* pat, *pot;
    
    tpl = generate_ball_template(radius,norm,exclude_center);
    float sigma = radius;
    float* weights = create_gaussian_weights(tpl,sigma);
    pat = alloc_patch(tpl->k);
    pot = alloc_patch(tpl->k);
    //
    // non-local means
    // search a window of size R
    //
    const int R = 100;
    const double h = 1.0;
    const int C = -0.5/(h*h);
    linear_template_t* ltpl = linearize_template(tpl,m,n);
    for (int i = 0, li = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j, ++li) {
            double y = 0; 
	    double n = 0;
            get_linear_patch(img,ltpl,i,j,NULL,pat);
            int di0 = i > R     ? i-R : 0;
            int di1 = i < (m-R) ? i+R : m;
            int dj0 = j > R     ? j-R : 0;
            int dj1 = j < (n-R) ? j+R : n;
            for ( int di = di0; di < di1; ++di ) {
                for ( int dj = dj0; dj < dj1; ++dj ) {
                    get_linear_patch(img,ltpl,di,dj,NULL,pot);
		    const double d = patch_dist(pat,pot,weights);
		    const double w = exp(C*d);
		    y += w*get_pixel(img,di,dj);
                    n += w;
                }
            }
	    const int x = (int) (0.5 + y/n);
            set_linear_pixel(&out,li,x);
	}
    }
    //
    //
    //
    snprintf(ofname,128,"nlm_%s",fname);
    int res = write_pnm(ofname,&out);
    if (res != RESULT_OK) {
        fprintf(stderr,"error writing image %s.\n",ofname);
    }
    free_patch(pot);
    free_patch(pat);
    free_linear_template(ltpl);
    free_template(tpl);
    pixels_free(img->pixels);
    pixels_free(out.pixels);
    free(img);
    free(weights);
    return res;
}
