#include "patch_mapper.h"
#include <assert.h>
#include <string.h>

/**
 * maps the samples of a patch onto a binary representation
 * using the words of the output patch as bit fields.
 * 
 * The binarized pixels are stored from MSB to LSB,
 * starting from the first sample in the output.
 * 
 * So if the size of pixel_t is, say, 8 bits, and we have patches of size, say, 3x3=9 pixels
 * we store the binarized patch samples x0,x1,...,x9 as follows:
 * 
 * | x0 x1 x2 ... x7 | x8 0 0 0 0 0 0 |
 * 
 * For example, let's say our template is a 3x3 template ordered by distance from the center:
 * 
 * 5 2 6
 * 1 0 3
 * 7 4 8
 * 
 * and we see the following  patch (which has been centered to 0)
 * 
 * 13 12 -1
 * 7  9  -5
 * -4 -4 -14
 * 
 * We have the following binarized samples sequence:
 * 
 * x = ( 1 1 1 0 0 1 0 0 0 )
 * 
 * and this gets mapped to the following bits in the output patch samples vector y
 * 
 *       y[0]                y[1]
 * | 1 1 1 0 0 1 0 0 | 0 0 0 0 0 0 0 0 |
 *   
 * that is 228, 0
 * 
 * if the size of the output patch is larger than 2, the rest of the samples are set to 0
 * 
 */ 
void binary_patch_mapper( const patch_t * in, patch_t * out ) {
    assert(sizeof(pixel_t) == sizeof(upixel_t));
    const index_t ki = in->k;
    const size_t s = 8*sizeof(upixel_t); // number of BITS that fit in a pixel_t/upixel_t
    const size_t ko = ki / s + (ki % s ? 1 : 0); // number of pixel_t samples required for output
    const upixel_t msb = 1 << (s-1); // MSB
    upixel_t mask = msb;
    const pixel_t* p = in->values;
    pixel_t* q = out->values;
    memset(q,0,sizeof(pixel_t)*out->k);
    for (int i = 0, j = 0; i < ki; ++i) {
        if (p[i] > 0) {
            q[j] |= mask;
        }
        // update binary mask
        // if it gets to 0, 
        mask >>= 1;
        if (!mask) {
            mask = msb; // start again from MSB
            ++j; // advance to next word 
        }
    }
}
