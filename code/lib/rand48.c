#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "rand48.h"
/* 48-bit numbers are stored as arrays of 3 32-bit longs, each holding
   16 bits of the number (the waste is on purpose, so we can multiply 
   16-bit numbers without overflow) */

typedef unsigned long ulong;

static ulong
	X[3] = { 0x0000330E, 0x00000001, 0x00000000 }, /* the state variable */
	A[3] = { 0x0000E66D, 0x0000DEEC, 0x00000005 }, /* the multiplier */
	C    = 0x0000000B;  /* the constant term */

#define SEED0 0x330E


static void mpy48(ulong *, ulong *, ulong *);

#ifdef STANDALONERAND48
#define SAMPLES 100000
#define BINS 20
main(int argc, char **argv)
{
    
    int i, seedval = 1;
    unsigned long x,y=0, samples = SAMPLES;
    unsigned long hist[BINS];
    double d, z;
    unsigned char *cc = (unsigned char *)(&d);

    for ( i=1; i<argc; i++ ) {
	if ( argv[i][0] == '-' )  {
	    switch ( argv[i][1] ) {
	    case 'n':
		    sscanf(argv[i],"-n%d",&samples);
		    break;
	    case 's':
		    sscanf(argv[i],"-s%d",&seedval);
		    break;
	    }
	}
    }

    srand48(seedval);

#if 0    /* debug */
    d = drand48();
    printf("%40.30e\n",d);
    for ( i=0; i<sizeof(d); i++ ) printf("%02x ",(unsigned)cc[i]);
    printf("\n");
    for ( i=0; i<3; i++ ) printf("X[%d] = %04x\n",i, X[i]);
    printf("\n");
    exit(10);
#endif


    for ( i=0; i<BINS; i++ ) hist[i] = 0;

    for ( i=0; i<samples; i++ ) {
	y += mrand48();
    }
    printf("seed = %d samples = %ld y = %08lx\n",seedval, samples, y);

    z = 0.0;
    for ( i=0; i<samples; i++ ) {
	d = drand48();
	z += d;
	x = (unsigned long)floor(BINS*d);
	hist[x] ++;
    }
    printf("z = %e\n",z);
    for ( i=0; i<BINS; i++ )
	printf("%3d %8ld %9.6e %9.6e\n",i,hist[i],(double)hist[i]/samples,(double)BINS*hist[i]/samples);


}
#endif

void 
srand48(long seedval)
{
    ulong s = (ulong)seedval;
    X[2] = (s>> 16);
    X[1] = s & 0xFFFF;
    X[0] = SEED0;
}

void 
update_buffer(void)
{
/* This function generates the same sequence as the HP-UX implementation */
    ulong cy;

    mpy48(X,A,X);

    X[0] += C;
    cy = X[0]&0x00010000;
    X[0] &= 0xFFFF;

    if ( cy ) {
	X[1]++;
	cy = X[1]&0x00010000;
	X[1] &= 0xFFFF;
	if ( cy ) {
	    X[2]++;
	    X[2] &= 0xFFFF;
	}
    }
}

long 
mrand48(void)
{
/* This function generates the same sequence as the HP-UX implementation */

    update_buffer();
    return (long)((X[2]<<16)+X[1]);
}

#define D_2_16 ((double)(1L<<16))
#define D_2_32 (D_2_16*D_2_16)
#define D_2_48 (D_2_32*D_2_16)

double 
drand48(void)
{
/* This function MAY NOT generate the same sequence as the HP-UX 
   implementation (TO DO) */
    double d;

    update_buffer();
    d = (double)X[2]*D_2_32 + (double)X[1]*D_2_16 + (double)X[0];
    return d/D_2_48;
}

static void
mpy48(ulong *X, ulong *A, ulong *Y)
/* multiplies 2 48-bit numbers mod 2^48 */
{
    ulong Y0, Y1, Y2;
    register ulong yh, p1, p2, p1l, p1h, p2l, p2h;

    Y0 = X[0]*A[0];
    yh = Y0 >>16;   /* high order 16 bits */
    Y0 &= 0xFFFF;     /* low order 16 bits */

    p1 = X[0]*A[1];
    p1l = p1 & 0xFFFF;
    p1h = p1 >> 16;

    p2 = X[1]*A[0];
    p2l = p2 & 0xFFFF;
    p2h = p2 >> 16;

    Y1 = yh + p1l + p2l;
    yh = (Y1 >> 16) + p1h + p2h;
    Y1 &= 0xFFFF;   /* low order 16 bits of second digit */

    Y2 = X[2]*A[0]+X[1]*A[1]+X[0]*A[2]+yh;  /* don't care about carry here */
    Y2 &= 0xFFFF;

    /* store result */
    Y[0] = Y0;
    Y[1] = Y1;
    Y[2] = Y2;
}

long 
lrand48(void)
{
    return (long)(((ulong)mrand48())>>1);
}
