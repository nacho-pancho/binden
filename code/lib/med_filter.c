#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "image.h"

#define MAXW 10

/*------------------------------------------------------------------------*/

static int clip ( int i, int min, int max ) {
    return i <= min ? min : ( i >= max ? max : i );
}

/*------------------------------------------------------------------------*/

static int pixel_comp ( const void * p1, const void * p2 ) {
    return *( ( pixel_t * ) p1 ) - *( ( pixel_t * ) p2 );
}

/*------------------------------------------------------------------------*/
void med ( const image_t * in, int w, image_t * out ) {

    const int ancho = in->info.width;
    const int alto =  in->info.height;
    const int wsz = ( w + 1 ) * ( w + 1 );
    const pixel_t * pi = in->pixels;
    pixel_t * po       = out->pixels;
    int i, j, di, dj, i2, j2, k;
    pixel_t buffer[ ( MAXW + 1 ) * ( MAXW + 1 ) ]; /* maximo tamaño de ventana */

    printf ( "ancho=%d alto=%d\n", ancho, alto );
    for ( i = 0 ; i < alto ; ++i ) {
        for ( j = 0 ; j < ancho ; ++j ) {
            k = 0;
            for ( di = -w ; di <= w ; ++di ) {
                for ( dj = -w ; dj <= w ; ++dj ) {
                    i2 = clip ( i + di, 0, alto - 1 );
                    j2 = clip ( j + dj, 0, ancho - 1 );
                    buffer[ k++ ] = pi[ i2 * ancho + j2 ];
                }
            }
            qsort ( buffer, wsz, sizeof( pixel_t ), pixel_comp );
            po[ i * ancho + j ] = ( wsz % 2 ) == 0 ?
                                  ( buffer[ wsz / 2 ] + buffer[ wsz / 2 - 1 ] )  / 2 :
                                  buffer[ wsz / 2 ];
        }
    }
}
