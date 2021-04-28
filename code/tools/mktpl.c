#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "image.h"
#include "ascmat.h"
#include "templates.h"


/*---------------------------------------------------------------------------------------*/
/* MAIN */
/*---------------------------------------------------------------------------------------*/
void parse_options(int* argc, char** argv[]);

/**
 * Utility for making patches of various shapes and sizes
 * Main parameters are radius and number of samples (size of the patch)
 * Those parameters are interpreted differently depending on the template construction method.
 * Usage
 * ./mktpl -rradius -ksize -ln
 * -rradius: maximum distance from center pixel, where the distance is given by the ln norm
 * -ksize  : number of samples in resulting template. 0 means cover the whole ln ball of radius r with samples. Mandatory for random template generation.
 * -ln     : use norm n for measuring the radius
 * -s      : for random templates, force symmetric pattern
 * -t: base template data. If this is a number, random templates are generated with this number as seed. Otherwise it is assummed to be an existing template file to use as source.
 * -n  : number of templates to be generated, used for random template generation
 * -mn : multiscale. For each base template, generate n-1 other ones by scaling the original offsets by 2,3,...,n
 */
const char* progname;
const char* tplfile = "1234";
const char* outfile = "-";
short maxscale = 1;
short maxradius = 5;
short symmetric = 0;
short tplsize = 3;
short norm = 2; // infinity norm
template_t* intpl;
template_t* outtpl;
int nin = 1;
int nout;

int main (int argc, char *argv[]) {
    parse_options(&argc,&argv);
    printf("argc=%d\n",argc);
    /* interpretacion de comandos de entrada */
    //
    // Base templates
    //
    if (isalpha(tplfile[0])) {
        read_template_multi(tplfile,&intpl,&nin);
        tplsize = intpl[0].k;
    } else {
      int par = atoi(tplfile);
      if (par == 0) { // dense 
	nin = nout = 1;
	alloc_template((2*maxradius+1)*(2*maxradius)-1);
	intpl = generate_ball_template(maxradius,norm,0);	
      } else {
        srand(par); // parameter is seed
        intpl = (template_t*) malloc(nin*sizeof(template_t));
        int t = 0;
        for (t = 0; t < nin ; t++) {
            ini_template(&intpl[t],tplsize);
            generate_random_template(maxradius,norm,tplsize,symmetric,0,&intpl[t]);
	    //            print_template(&intpl[t]);
        }
      }
    }
    //
    // symmetrize
    //
    outtpl = intpl;
    nout = nin;
    if (symmetric) {
      printf("Symmetrizing\n");
        outtpl = (template_t*) malloc(nin*sizeof(template_t));
	int t;
	int maxk = 0;
        for (t = 0; t < nin ; t++) {
	  ini_template(&outtpl[t],4*tplsize);
	  symmetrize_template(intpl+t,outtpl+t);
	  //	  print_template(&outtpl[t]);
	  if (maxk < outtpl[t].k) 
	    maxk = outtpl[t].k;
	}
        for (t = 0; t < nin ; t++) {
	  outtpl[t].k = maxk;
	}
    }
    //
    // multiscale
    //
    if (maxscale > 1) {
      printf("Scaling\n");
      if (outtpl != intpl) {
	int t;
	for (t=0; t < nin; t++) 
	  destroy_template(intpl+t);
	free(intpl);
	intpl = outtpl;
      }
      nout = nin*maxscale;
      outtpl = (template_t*) malloc(nout*sizeof(template_t));	
      int t;
      for (t=0; t < nin; t++) {
	int sc;
	for (sc = 1; sc <= maxscale; sc++) {
	  int tout = maxscale*t+(sc-1);
	  printf("k=%d t=%d sc=%d tout=%d nout=%d\n",intpl[0].k,t,sc,tout,nout);
	  ini_template(&outtpl[tout],intpl[t].k);
	  int k;
	  for (k = 0; k < intpl[t].k; k++) {
	    outtpl[tout].coords[k].i = sc * intpl[t].coords[k].i;
	    outtpl[tout].coords[k].j = sc * intpl[t].coords[k].j;	    
	  }
	}
      }      
    }
    int t;
    for (t = 0; t < nout; ++t) {
      printf("template_t %d\n",t);
      print_template(&outtpl[t]);
    }
    FILE* f;
    if (outfile[0] == '-') {
      f = stdout;
    } else {
      f = fopen(outfile,"w");
    }
    for (t = 0; t < nout ; t++) {
      dump_template(&outtpl[t],f);
      if (outtpl != intpl) {
	destroy_template(outtpl+t);
      }
    }
    for (t = 0; t < nin ; t++) {
      destroy_template(intpl+t);
    }
    if (outtpl != intpl) free(outtpl);
    free(intpl);
    if (outfile[0] != '-') 
      fclose(f);
    return 0;
}

/*---------------------------------------------------------------------------------------*/

/**
 * Utility for making patches of various shapes and sizes
 * Main parameters are radius and number of samples (size of the patch)
 * Those parameters are interpreted differently depending on the template construction method.
 * Usage
 * ./mktpl -rradius -ksize -ln
 * -rradius: maximum distance from center pixel, where the distance is given by the ln norm
 * -ksize  : number of samples in resulting template. 0 means cover the whole ln ball of radius r with samples. Mandatory for random template generation.
 * -ln     : use norm n for measuring the radius
 * -s      : for random templates, force symmetric pattern
 * -t: base template data. If this is a number, random templates are generated with this number as seed. Otherwise it is assummed to be an existing template file to use as source.
 * -nn  : number of templates to be generated, used for random template generation
 * -mn : multiscale. For each base template, generate n-1 other ones by scaling the original offsets by 2,3,...,n
 */
void parse_options(int* pargc, char** pargv[]) {
    int argc = *pargc;
    char** argv = *pargv;
    progname = argv[0];
    for (++argv,--argc; (argc > 0) && *argv[0] == '-'; --argc, ++argv) {
        switch (argv[0][1]) {
        case 't':
            if (argv[0][2]) // parameter val is glued , -tfilename
                tplfile = argv[0]+2;
            else if (argc-- > 0) // parameter val is next arg
                tplfile = (++argv)[0];
            printf("tplfile=%s\n",tplfile);
            break;
        case 'o':
            if (argv[0][2]) // parameter val is glued , -tfilename
                outfile = argv[0]+2;
            else if (argc-- > 0) // parameter val is next arg
                outfile = (++argv)[0];
            printf("outfile=%s\n",outfile);
            break;
        case 'l':
            if (argv[0][2]) // parameter val is glued , -pval
                norm = atoi(argv[0]+2);
            else if (argc-- > 0) // parameter val is next arg
                norm = atoi((++argv)[0]);
            printf("norm=%d\n",norm);
            break;
        case 'n':
            if (argv[0][2]) // parameter val is glued , -pval
                nin = atoi(argv[0]+2);
            else if (argc-- > 0) // parameter val is next arg
                nin = atoi((++argv)[0]);
            printf("ntemplates=%d\n",nin);
            break;
        case 'k':
            if (argv[0][2]) // parameter val is glued , -pval
                tplsize = atoi(argv[0]+2);
            else if (argc-- > 0) // parameter val is next arg
                tplsize = atoi((++argv)[0]);
            printf("tplsize=%d\n",tplsize);
            break;
        case 's':
	    symmetric = 1;
            printf("symmetric templates=true\n");
            break;
        case 'r':
            if (argv[0][2]) // parameter val is glued , -pval
                maxradius = atoi(argv[0]+2);
            else if (argc-- > 0) // parameter val is next arg
                maxradius = atoi((++argv)[0]);
            printf("maxradius=%d\n",maxradius);
            break;
        case 'm':
            if (argv[0][2]) // parameter val is glued , -pval
                maxscale = atoi(argv[0]+2);
            else if (argc-- > 0) // parameter val is next arg
                maxscale = atoi((++argv)[0]);
            printf("maxscale=%d\n",maxscale);
            break;
        default:
            printf("Unknown option %s\n",argv[0]);
        }
    }
    *pargc = argc;
    *pargv = argv;
}

#if 0
/*---------------------------------------------------------------------------------------*/

template_t* ini_template(template_t* pt, int k) {
    if (pt == NULL) {
      pt = (template_t*) calloc(sizeof(template_t),1);
    }
    pt->k = k;
    pt->is = (index_t*) calloc(sizeof(index_t),k);
    pt->js = (index_t*) calloc(sizeof(index_t),k);
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

template_t* generate_ball_template(int radius, int norm, template_t* pt) {
  int i,j;
  int k = 0;
  for (i = -radius; i <= radius; i++) {
    for (j = -radius; j <= radius; j++) {
      if ((i==0) && (j==0)) continue;
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
      if (rad <= radius) {
	pt->is[k] = i;
	pt->js[k++] = j;
      }
    }
  }
  pt->k = k;
  return pt;
}

/*---------------------------------------------------------------------------------------*/

template_t* generate_random_template(int radius, int norm, int k, int sym, template_t* pt) {
    int r;
    if (pt == NULL) {
      pt = ini_template(NULL,sym? 4*k:k);
    }
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
	if ((rad == 0) || (rad > radius)) {
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
    //
    // symmetrize
    //
    return pt;
}

/*---------------------------------------------------------------------------------------*/

void symmetrize_template(const template_t* in, template_t* out) {
  const int k = in->k;
  if (out == NULL) {
    out = ini_template(NULL,4*k);
  }
  int r, sk = k;
  for (r=0 ; r<k ; r++) {
    const int i = in->is[r];
    const int j = in->js[r];
    out->is[r] = i;
    out->js[r] = j;
    if ((i != 0) && (j != 0)) { // i != 0, j != 0
      out->is[sk]   = -i;
      out->js[sk++] =  j;
      out->is[sk]   =  i;
      out->js[sk++] = -j;
    }
    out->is[sk] = -i;
    out->js[sk++] = -j;
  }
  out->k = sk;
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

void dump_template(const template_t* ptpl,FILE* ft) {
    int k;
    for (k = 0; k < ptpl->k; k++) {
      fprintf(ft,"%3d ",ptpl->is[k]);
    }
    fputc('\n',ft);
    for (k = 0; k < ptpl->k; k++) {
      fprintf(ft,"%3d ",ptpl->js[k]);
    }
    fputc('\n',ft);
}

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
#endif