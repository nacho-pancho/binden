#include "bitfun.h"

/* information on how to vectorize these operations using x86 SSE extensions in C is
   described in https://gcc.gnu.org/onlinedocs/gcc/Vector-Extensions.html */

/* https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetTable */

/*
 * very efficient general purpose method for counting bits w/o requiring
 * a look up table. An alternative is a LUT as a 32 bit block where one has 16 2 bit
 * counts for every of the 16 possible 4 bit nibbles.
 *
 * There is a POPCNT CPU extension in intel-base 64 bit processors since 2014,
 * but I don't want to be compiler dependent for now.
 */
//
// these functions are built in in GNU C
//
#ifndef __GNUC__

static idx_t block_weight_gen ( const block_t& v ) {
    static const unsigned char byte_weight_table[ 256 ] = {
#   define B2( n ) n,     n + 1,     n + 1,     n + 2
#   define B4( n ) B2 ( n ), B2 ( n + 1 ), B2 ( n + 1 ), B2 ( n + 2 )
#   define B6( n ) B4 ( n ), B4 ( n + 1 ), B4 ( n + 1 ), B4 ( n + 2 )
        B6 ( 0 ), B6 ( 1 ), B6 ( 1 ), B6 ( 2 )
    };
    const unsigned char * p = ( const unsigned char * ) &v;
    register idx_t w = 0;
    for ( idx_t i = 0 ; i < sizeof( block_t ) ; ++i ) {
        w += byte_weight_table[ p[ i ] ];
    }
    return w;
}

/* parity computation, much faster than sum */
static bool block_sum_gen ( const block_t& v ) {
    static const bool byte_parity_table[ 256 ] = {
#   define P2( n ) n, n ^ 1, n ^ 1, n
#   define P4( n ) P2 ( n ), P2 ( n ^ 1 ), P2 ( n ^ 1 ), P2 ( n )
#   define P6( n ) P4 ( n ), P4 ( n ^ 1 ), P4 ( n ^ 1 ), P4 ( n )
        P6 ( 0 ), P6 ( 1 ), P6 ( 1 ), P6 ( 0 )
    };
    const unsigned char * p = ( unsigned char * ) &v;
    idx_t ti = 0;
    #pragma omp parallel for
    for ( idx_t i = 0 ; i < sizeof( block_t ) ; ++i ) {
        ti ^= p[ i ];
    }
    return byte_parity_table[ ti ];
    //  return ParityTable256[tip[0] ^ p[1] ^ p[2] ^ p[3]];
}
#endif
//
// ifndef __GNUC__
//
