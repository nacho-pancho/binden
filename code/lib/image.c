#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "image.h"

/*---------------------------------------------------------------------------------------*/

pixel_t * pixels_alloc ( const image_info_t * info ) {
    const int npixels = info->channels * info->width * info->height;
    assert ( npixels > 0 );
    return ( pixel_t * ) malloc ( npixels * sizeof( pixel_t ) );
}
//
//---------------------------------------------------------------------------------------------
//
void pixels_free ( pixel_t * pix ) {
    if ( pix ) {
        free ( pix );
    }
}

pixel_t * pixels_copy ( const image_info_t * info, const pixel_t* src ) {
    const int npixels = info->channels * info->width * info->height;
    assert ( npixels > 0 );
    pixel_t* dest = pixels_alloc ( info );
    memcpy ( dest, src, npixels * sizeof( pixel_t ) );
    return dest;
}

image_t* image_copy ( const image_t * src ) {
    image_t* dest = (image_t*) calloc(1,sizeof(image_t));
    memcpy(dest,src,sizeof(image_t));
    dest->pixels = pixels_copy(&src->info, src->pixels);
    return dest;
}

void pixels_copyto ( image_t* dest, const image_t * src ) {
    const image_info_t* info = &src->info;
    const int srcpixels = info->channels * info->width * info->height;
    const int destpixels = dest->info.channels * dest->info.width * dest->info.height;
    assert ( srcpixels > 0 );
    assert ( srcpixels == destpixels) ;
    memcpy ( dest->pixels, src->pixels, srcpixels * sizeof( pixel_t ) );
}

/*---------------------------------------------------------------------------------------*/

int get_pixel ( const image_t * pimg, const int i, const int j ) {
    const image_info_t info = pimg->info;
    return ( i < 0 ) || ( i >= info.height ) || ( j < 0 ) || ( j >= info.width ) ? 0 : pimg->pixels[ i * info.width + j ];
}

/*---------------------------------------------------------------------------------------*/

int get_linear_pixel ( const image_t * pimg, const int li ) {
    const int npixels = pimg->info.width * pimg->info.height;
    return ( li < 0 ) || ( li >= npixels ) ? 0 : pimg->pixels[ li ];
}

/*---------------------------------------------------------------------------------------*/

void set_pixel ( image_t * pimg, const int i, const int j, const pixel_t val ) {
    const image_info_t info = pimg->info;
    assert ( ( i >= 0 ) && ( i < info.height ) && ( j >= 0 ) && ( j < info.width ) );
    pimg->pixels[ i * info.width + j ] = val;
}

/*---------------------------------------------------------------------------------------*/

void set_linear_pixel ( image_t * pimg, const int li, const pixel_t val ) {
    const int npixels = pimg->info.width * pimg->info.height;
    assert ( ( li >= 0 ) && ( li < npixels ) );
    pimg->pixels[ li ] = val;
}
