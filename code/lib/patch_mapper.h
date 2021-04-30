/**
 * Patch mappers are functions that take a patch and transform it
 * to another representation.
 * Examples of these might be alignment, dimensionality reduction, etc.
 *
 * They are meant to be lightweight and very fast, so that the size
 * of the destination patch must be able to hold its size.
 * 
 * \brief Some basic patch mappers for using when extracting patches.
 * \file patch_mapper.h 
 */
#ifndef PATCH_MAPPER_H
#define PATCH_MAPPER_H

#include "patches.h"

size_t compute_binary_mapping_samples(const int k);

/**
 * packs the patch samples into to a binary representation
 * using 1 bit per sample. The MSB holds the first sample.
 * If the input patch has size K, the destination patch must be able to hold
 * K/bpp, where bpp is the width in bytes of the type pixel_t
 */
void binary_patch_mapper( const patch_t * orig_patch, patch_t * mapped_patch );

#endif