#ifndef BIT_FUN_H
#define BIT_FUN_H

#include "types.h"
#define block_t upixel_t

#ifdef __GNUC__
#define block_weight( a ) __builtin_popcountl ( a )
#else
#define block_weight( a ) block_weight_gen ( a )
#endif


#ifdef __GNUC__
#define block_sum( a ) __builtin_parityl ( a )
#else
#define block_sum( a ) block_sum_gen ( a )
#endif

#define BITS_PER_BLOCK ( sizeof( block_t ) * 8 )
#define ONES   ( ~block_t ( 0 ) )
#define ZEROES ( block_t ( 0 ) )
#define LSB    block_t ( 1 )
#define MSB    ( LSB << ( BITS_PER_BLOCK - 1 ) )
#define IMSB   ( ONES >> 1 )
#define ILSB   ( ONES << 1 )

#define XOR( a, b ) ( ( !( a ) && ( b ) ) ||  ( ( a ) && !( b ) ) )

#endif