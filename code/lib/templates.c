#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "templates.h"
#include "ascmat.h"
/*---------------------------------------------------------------------------------------*/

linear_template_t* linearize_template(const template_t* pt, int nrows, int ncols) {
    linear_template_t* plt = alloc_linear_template(pt->k);
    const int k = pt->k;
    for (int r=0; r<k; ++r) {
        plt->li[r] = pt->coords[r].i*ncols+pt->coords[r].j;
    }
    return plt;
}

/*---------------------------------------------------------------------------------------*/
linear_template_t* alloc_linear_template(int maxk) {
    linear_template_t* pt = (linear_template_t*) malloc(sizeof(linear_template_t));
    pt->k = maxk;
    pt->li = (index_t*) malloc(sizeof(index_t)*maxk);
    return pt;
}

/*---------------------------------------------------------------------------------------*/

template_t* alloc_template(int maxk) {
    template_t* pt = (template_t*) malloc(sizeof(template_t));
    pt->k = maxk;
    pt->coords = (coord_t*) malloc(sizeof(coord_t)*maxk);
    return pt;
}

/*---------------------------------------------------------------------------------------*/

void free_template(template_t* pt) {
    if (pt == NULL) return;
    pt->k = 0;
    free(pt->coords);
    free(pt);
}
/*---------------------------------------------------------------------------------------*/

void free_linear_template(linear_template_t* pt) {
    if (pt == NULL) return;
    pt->k = 0;
    free(pt->li);
    free(pt);
}


/*---------------------------------------------------------------------------------------*/

template_t* generate_uniform_random_template(int max_l1_radius, int k, int exclude_center) {
    int r;
    template_t* pt = alloc_template(k);
    for (r=0 ; r<k ; r++) {
        // sample i and j
        int i = (int)(2.*(double)max_l1_radius*(double)rand()/(double)RAND_MAX - (double)max_l1_radius+ 0.5);
        int j = (int)(2.*(double)max_l1_radius*(double)rand()/(double)RAND_MAX - (double)max_l1_radius+ 0.5);
        if (exclude_center && !i && !j) {
            r--;
            continue;
        }
        // see if not already there
        int r2 ;
        for (r2 = 0; r2 < r; r2++) {
            if ((i==pt->coords[r2].i) && (j==pt->coords[r2].j)) {
                break;
            }
        }
        if (r2 < r) {
            r--;
            continue;
        }
        pt->coords[r].i = i;
        pt->coords[r].j = j;
    }
    return pt;
}

/*---------------------------------------------------------------------------------------*/

template_t* generate_random_template(int radius, int norm, int k, int sym, int exclude_center) {
    int r;
    template_t* pt = alloc_template(sym? 4*k:k);
    for (r=0 ; r<k ; r++) {
        // this generates a sample within the linfinity ball
        // 
      int i,j;
      if (!sym) {
        i = (int)(2.*(double)radius*(double)rand()/(double)RAND_MAX - (double)radius+ 0.5);
        j = (int)(2.*(double)radius*(double)rand()/(double)RAND_MAX - (double)radius+ 0.5);
      } else {
        i = (int)((double)radius*(double)rand()/(double)RAND_MAX+ 0.5);
        j = (int)((double)radius*(double)rand()/(double)RAND_MAX+ 0.5);       
      }
	double rad;
      int ai = i >= 0 ? i : -i;
      int aj = j >= 0 ? j : -j;
      if (norm >= 1000) {
	rad = ai > aj ? ai : aj;
      } else if (norm == 1) {
	rad = ai + aj;
      } else {
	rad = pow(pow((double)i,(double)norm) + pow((double)j,(double)norm),1.0/(double)norm);
      }
	if ((exclude_center && (rad == 0)) || (rad > radius)) {
            r--;
            continue;
        } 
        // see if not already there
        int r2 ;
        for (r2 = 0; r2 < r; r2++) {
            if ((i==pt->coords[r2].i) && (j==pt->coords[r2].j)) {
                break;
            }
        }
        if (r2 < r) {
            r--;
            continue;
        }
        pt->coords[r].i = i;
        pt->coords[r].j = j;
    }
    //
    // symmetrize
    //
    return pt;
}

/*---------------------------------------------------------------------------------------*/

template_t* read_template(const char* fname) {
    double* tpldat  = 0;
    unsigned int nrows = 0;
    unsigned int ncols = 0;
    template_t* pt;
    read_ascii_matrix(fname,&nrows,&ncols,&tpldat);
    printf("TEMPLATE (k=%d):\n",ncols);
    print_ascii_matrix(nrows,ncols,tpldat);
    pt = alloc_template(ncols);
    for (unsigned r = 0; r < ncols; ++r) {
        pt->coords[r].i = tpldat[r];
        pt->coords[r].j = tpldat[ncols+r];
    }
    free(tpldat);
    return pt;
}


/*---------------------------------------------------------------------------------------*/

void print_template(const template_t* ptpl) {
    int min_i = 10000,min_j = 10000, max_i=-10000, max_j=-10000;
    int k,r,l;
    for (k = 0; k < ptpl->k; k++) {
        if (ptpl->coords[k].i < min_i) min_i = ptpl->coords[k].i;
        if (ptpl->coords[k].i > max_i) max_i = ptpl->coords[k].i;
        if (ptpl->coords[k].j < min_j) min_j = ptpl->coords[k].j;
        if (ptpl->coords[k].j > max_j) max_j = ptpl->coords[k].j;
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
                if ((ptpl->coords[l].i == (min_i+k)) && (ptpl->coords[l].j == (min_j+r))) {
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

/*---------------------------------------------------------------------------------------*/

void dump_template(const template_t* ptpl,FILE* ft) {
    int k;
    for (k = 0; k < ptpl->k; k++) {
      fprintf(ft,"%3d ",ptpl->coords[k].i);
    }
    fputc('\n',ft);
    for (k = 0; k < ptpl->k; k++) {
      fprintf(ft,"%3d ",ptpl->coords[k].j);
    }
    fputc('\n',ft);
}

/*---------------------------------------------------------------------------------------*/

template_t* symmetrize_template(const template_t* in) {
  const int k = in->k;
  template_t* out = alloc_template(4*k);
  int r, sk = k;
  for (r=0 ; r<k ; r++) {
    const int i = in->coords[r].i;
    const int j = in->coords[r].j;
    out->coords[r].i = i;
    out->coords[r].j = j;
    if ((i != 0) && (j != 0)) { // i != 0, j != 0
      out->coords[sk].i   = -i;
      out->coords[sk++].j =  j;
      out->coords[sk].i   =  i;
      out->coords[sk++].j = -j;
    }
    out->coords[sk].i = -i;
    out->coords[sk++].j = -j;
  }
  out->k = sk;
  return out;
}

/*---------------------------------------------------------------------------------------*/

template_t* generate_ball_template(int radius, int norm, int exclude_center) {
  assert(radius > 0);
  template_t* pt = alloc_template((2*radius+1)*(2*radius+1)); // largest possible context 
  int i,j;
  int k = 0;
  for (i = -radius; i <= radius; i++) {
    for (j = -radius; j <= radius; j++) {
      if (exclude_center && (i==0) && (j==0)) continue;
      double rad;
      int ai = i >= 0 ? i : -i;
      int aj = j >= 0 ? j : -j;
      if (norm >= 1000) {
	rad = ai > aj ? ai : aj;
      } else if (norm == 1) {
	rad = ai + aj;
      } else {
	rad = pow( pow((double)ai,(double)norm) + pow((double)aj,(double)norm), 1.0/(double)norm );
      }
      if (rad <= radius) {
	pt->coords[k].i = i;
	pt->coords[k++].j = j;
      }
    }
  }
  pt->k = k;
  return pt;
}

/*---------------------------------------------------------------------------------------*/
#if 0
static int compare_coords_r(const void* pa, const void* pb, void* pnorm) {
    const coord_t* a = (coord_t*) pa;
    const coord_t* b = (coord_t*) pb;
    const int norm = *((int*) pnorm);
    //
    // first compare norms: smaller goes first
    //
	const double na = 
        pow( fabs((double)a->i), (double)norm ) + 
        pow( fabs((double)a->j),(double)norm );
	const double nb = 
        pow( fabs((double)b->i), (double)norm ) + 
        pow( fabs((double)b->j),(double)norm );
    if (na < nb) {
        return -1;
    } else if (na > nb) {
        return 1;
    } else {
        // compare angles
        const double aa = atan2((double)a->j,(double)a->i);
        const double ab = atan2((double)b->j,(double)b->i);
        return (aa < ab) ? -1: ((aa > ab) ? 1: 0 );
    }
}
#endif

static int compare_coords(const void* pa, const void* pb) {
    const coord_t* a = (coord_t*) pa;
    const coord_t* b = (coord_t*) pb;
    const int norm = 2;
    //
    // first compare norms: smaller goes first
    //
	const double na = 
        pow( fabs((double)a->i), (double)norm ) + 
        pow( fabs((double)a->j),(double)norm );
	const double nb = 
        pow( fabs((double)b->i), (double)norm ) + 
        pow( fabs((double)b->j),(double)norm );
    if (na < nb) {
        return -1;
    } else if (na > nb) {
        return 1;
    } else {
        // compare angles
        const double aa = atan2((double)a->j,(double)a->i);
        const double ab = atan2((double)b->j,(double)b->i);
        return (aa < ab) ? -1: ((aa > ab) ? 1: 0 );
    }
}

template_t* sort_template(template_t* orig, int in_place) {
    template_t* out;
    if (in_place) {
        out = orig;
    } else {
        out = alloc_template(orig->k);
        memcpy(out->coords,orig->coords,orig->k*sizeof(coord_t));
    }
    qsort(out->coords,out->k,sizeof(coord_t),compare_coords);
    return out;
}
