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