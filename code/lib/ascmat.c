#include <ctype.h>
#include <stdlib.h>

#include "ascmat.h"

int scan_ascii_matrix ( const char * fname, index_t * pnrows, index_t * pncols ) {
    char spc;
    int c;
    FILE * f;
    f = fopen ( fname, "r" );
    if ( f == NULL ) {
        return -1;
    }
    // count columns
    spc = 1;
    *pncols = 0;
    *pnrows = 0;
    do {
        c = fgetc ( f );
        if ( isspace ( c ) ) {
            spc = 1;
        } else {
            if ( c == '#' ) { // a comment begins
                if ( *pncols > 0 )
                    break;
                else {
                    // all lines so far have been pure comments, no data
                    while ( ( c = fgetc ( f ) ) != '\n' && c != EOF );
                    if ( c == EOF ) // file has nothing but comments
                        break;
                    // data may come in lines to be read next
                    c = ' '; // avoid finishing
                    continue;
                }
            }
            if ( spc ) { // was space, now it isn't
                spc = 0;
                ( *pncols )++;
            }
        }
    } while ( c != '\n' && c != EOF );
    // now count rows
    if ( *pncols == 0 ) {
        fclose ( f );
        return 0;
    }
    printf ( "n=%ld\n", *pncols );
    *pnrows = 1;
    int empty = 1;
    while ( ( c = fgetc ( f ) ) != EOF ) {
        if ( !isspace ( c ) && c != '#' )
            empty = 0;
        if ( !empty && ( c == '\n' ) ) ( *pnrows )++;
    }
    printf ( "m=%ld\n", *pnrows );
    fclose ( f );
    return 0;
}

int read_ascii_matrix ( const char * fname, index_t * pnrows, index_t * pncols, double * * matdata ) {
    int res;
    res = scan_ascii_matrix ( fname, pnrows, pncols );
    if ( res ) return res;
    *matdata = ( double * ) malloc ( ( *pnrows ) * ( *pncols ) * sizeof( double ) );
    read_ascii_data ( fname, *pnrows, *pncols, *matdata );
    return res;
}

//
//======================================================================
//
int read_ascii_data ( const char * fname, const index_t nrows, const index_t ncols, double * data ) {
    FILE * in;
    in = fopen ( fname, "r" );
    if ( !in ) return -1;
    const index_t n = nrows * ncols;
    index_t i, r;
    float x;
    for ( i = 0 ; i < n ; ++i ) {
        r = fscanf ( in, "%f", &x );
        if ( r < 1 ) {
            fclose ( in );
            return -1;
        }
        data[ i ] = x;
    }
    fclose ( in );
    return 0;
}

void print_ascii_matrix ( const index_t nrows, const index_t ncols, const double * data ) {
    index_t i, j;
    for ( i = 0 ; i < nrows ; i++ ) {
        for ( j = 0 ; j < ncols ; j++ ) {
            printf ( "%7.4f ", data[ i * ncols + j ] );
        }
        putchar ( '\n' );
    }
}
