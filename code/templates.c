#include <stdlib.h>
#include "templates.h"
#include "ascmat.h"
/*---------------------------------------------------------------------------------------*/

void linearize_template(const template_t* pt, int nrows, int ncols, const linear_template_t* plt) {
    const int k = pt->k;
    int j;
    for (j=0; j<k; ++j) {
        plt->li[j] = pt->is[j]*ncols+pt->js[j];
    }
}

/*---------------------------------------------------------------------------------------*/

template_t* ini_template(template_t* pt, int k) {
    if (pt == NULL) {
        pt = (template_t*) malloc(sizeof(template_t));
    }
    pt->k = k;
    pt->is = (index_t*) malloc(sizeof(index_t)*k);
    pt->js = (index_t*) malloc(sizeof(index_t)*k);
    return pt;
}

/*---------------------------------------------------------------------------------------*/

void destroy_template(template_t* pt) {
    if (pt == NULL) return;
    pt->k = 0;
    free(pt->is);
    free(pt->js);
}

/*---------------------------------------------------------------------------------------*/

template_t* generate_random_template(int max_l1_radius, int k, template_t* pt) {
    int r;
    if (pt == NULL) {
        pt = ini_template(NULL,k);
    }
    for (r=0 ; r<k ; r++) {
        // sample i and j
        int i = (int)(2.*(double)max_l1_radius*(double)rand()/(double)RAND_MAX - (double)max_l1_radius+ 0.5);
        int j = (int)(2.*(double)max_l1_radius*(double)rand()/(double)RAND_MAX - (double)max_l1_radius+ 0.5);
        if (i*j == 0) {
            r--;
            continue;
        }
        // see if not already there
        int r2 ;
        for (r2 = 0; r2 < r; r2++) {
            if ((i==pt->is[r2]) && (j==pt->js[r2])) {
                break;
            }
        }
        if (r2 < r) {
            r--;
            continue;
        }
        pt->is[r] = i;
        pt->js[r] = j;
    }
    return pt;
}

/*---------------------------------------------------------------------------------------*/

void read_template(const char* fname, template_t* ptpl) {
    double* tpldat  = 0;
    unsigned nrows = 0;
    unsigned ncols = 0;
    int i;
    read_ascii_matrix(fname,&nrows,&ncols,&tpldat);
    printf("TEMPLATE (k=%d):\n",ncols);
    print_ascii_matrix(nrows,ncols,tpldat);
    ini_template(ptpl,ncols);
    for (i = 0; i < ncols; i++) {
        ptpl->is[i] = tpldat[i];
        ptpl->js[i] = tpldat[ncols+i];
    }
    free(tpldat);
}

/*---------------------------------------------------------------------------------------*/

void read_template_multi(const char* fname, template_t** ptpls, int* pntpl) {
    double* tpldat  = 0;
    unsigned nrows = 0;
    unsigned ncols = 0;
    template_t* tpls;
    int i,t;
    read_ascii_matrix(fname,&nrows,&ncols,&tpldat);
    //print_ascii_matrix(nrows,ncols,tpldat);
    int ntemplates = nrows / 2;
    tpls = (template_t*) malloc(sizeof(template_t)*ntemplates);
    for (t = 0; t < ntemplates; t++) {
        printf("TEMPLATE %d (k=%d):\n",t,ncols);
        ini_template(&tpls[t],ncols);
        for (i = 0; i < ncols; i++) {
            const int ii = tpls[t].is[i] = tpldat[2*t*ncols+i];
            const int ji = tpls[t].js[i] = tpldat[(2*t+1)*ncols+i];
	    if ((ii==0) && (ji==0)) {
                tpls[t].k = i;
                break;
            }
        }
        print_template(&tpls[t]);
    }
    free(tpldat);
    *ptpls = tpls;
    *pntpl = ntemplates;
}

/*---------------------------------------------------------------------------------------*/
void print_template(const template_t* ptpl) {
    int min_i = 10000,min_j = 10000, max_i=-10000, max_j=-10000;
    int k,r,l;
    for (k = 0; k < ptpl->k; k++) {
        if (ptpl->is[k] < min_i) min_i = ptpl->is[k];
        if (ptpl->is[k] > max_i) max_i = ptpl->is[k];
        if (ptpl->js[k] < min_j) min_j = ptpl->js[k];
        if (ptpl->js[k] > max_j) max_j = ptpl->js[k];
    }
    int a = max_i - min_i;
    int b = max_j - min_j;
    printf("    |");
    for (r = 0; r <= b; r++) {
        printf(" %2d ",r+min_j);
    }
    printf("\n----+");
    for (r = 0; r <= b; r++) {
        printf("----");
    }
    putchar('\n');
    for (k = 0; k <= a; k++) {
        printf(" %2d |",min_i+k);
        for (r = 0; r <= b; r++) {
            for (l=0; l < ptpl->k; l++) {
                if ((ptpl->is[l] == (min_i+k)) && (ptpl->js[l] == (min_j+r))) {
                    printf(" %2d ",l);
                    break;
                }
            }
            if (l == ptpl->k)
                printf("    ");
        }
        putchar('\n');
    }

}
