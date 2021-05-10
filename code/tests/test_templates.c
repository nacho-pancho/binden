#include <stdio.h>
#include <stdlib.h>

#include "pnm.h"
#include "image.h"
#include "templates.h"

int main ( int argc, char* argv[] ) {
    if ( argc < 2 ) {
        fprintf ( stderr, "usage: %s <radius> <norm>.\n", argv[ 0 ] );
        return RESULT_ERROR;
    }
    const int radius = atoi ( argv[ 1 ] );
    const int norm = atoi ( argv[ 2 ] );
    patch_template_t* tpl;
    tpl = generate_ball_template ( radius, norm, 1 );
    sort_template ( tpl, 1 ); // sorted by norm
    print_template ( tpl );
    free_patch_template ( tpl );
    return 0;
}
