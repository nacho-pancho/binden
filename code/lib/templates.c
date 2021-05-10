#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "templates.h"
#include "ascmat.h"
/*---------------------------------------------------------------------------------------*/

linear_template_t * linearize_template ( const patch_template_t * pt, index_t nrows, index_t ncols ) {
    linear_template_t * plt = alloc_linear_template ( pt->k );
    const index_t k = pt->k;
    for ( index_t r = 0 ; r < k ; ++r ) {
        plt->li[ r ] = pt->coords[ r ].i * ncols + pt->coords[ r ].j;
    }
    return plt;
}

/*---------------------------------------------------------------------------------------*/
linear_template_t * alloc_linear_template ( index_t maxk ) {
    linear_template_t * pt = ( linear_template_t * ) malloc ( sizeof( linear_template_t ) );
    pt->k = maxk;
    pt->li = ( index_t * ) malloc ( sizeof( index_t ) * maxk );
    return pt;
}

/*---------------------------------------------------------------------------------------*/

patch_template_t * alloc_patch_template ( index_t maxk ) {
    patch_template_t * pt = ( patch_template_t * ) malloc ( sizeof( patch_template_t ) );
    pt->k = maxk;
    pt->coords = ( coord_t * ) malloc ( sizeof( coord_t ) * maxk );
    return pt;
}

/*---------------------------------------------------------------------------------------*/

void free_patch_template ( patch_template_t * pt ) {
    if ( pt == NULL ) return;
    pt->k = 0;
    free ( pt->coords );
    free ( pt );
}
/*---------------------------------------------------------------------------------------*/

void free_linear_template ( linear_template_t * pt ) {
    if ( pt == NULL ) return;
    pt->k = 0;
    free ( pt->li );
    free ( pt );
}


/*---------------------------------------------------------------------------------------*/

patch_template_t * generate_uniform_random_template ( index_t max_l1_radius, index_t k, index_t exclude_center ) {
    index_t r;
    patch_template_t * pt = alloc_patch_template ( k );
    for ( r = 0 ; r < k ; r++ ) {
        // sample i and j
        index_t i = ( index_t ) ( 2. * ( double ) max_l1_radius * ( double ) rand( ) / ( double ) RAND_MAX - ( double ) max_l1_radius + 0.5 );
        index_t j = ( index_t ) ( 2. * ( double ) max_l1_radius * ( double ) rand( ) / ( double ) RAND_MAX - ( double ) max_l1_radius + 0.5 );
        if ( exclude_center && !i && !j ) {
            r--;
            continue;
        }
        // see if not already there
        index_t r2;
        for ( r2 = 0 ; r2 < r ; r2++ ) {
            if ( ( i == pt->coords[ r2 ].i ) && ( j == pt->coords[ r2 ].j ) ) {
                break;
            }
        }
        if ( r2 < r ) {
            r--;
            continue;
        }
        pt->coords[ r ].i = i;
        pt->coords[ r ].j = j;
    }
    return pt;
}

/*---------------------------------------------------------------------------------------*/

patch_template_t * generate_random_template ( index_t radius, index_t norm, index_t k, index_t sym, index_t exclude_center ) {
    index_t r;
    patch_template_t * pt = alloc_patch_template ( sym ? 4 * k : k );
    for ( r = 0 ; r < k ; r++ ) {
        // this generates a sample within the linfinity ball
        //
        index_t i, j;
        if ( !sym ) {
            i = ( index_t ) ( 2. * ( double ) radius * ( double ) rand( ) / ( double ) RAND_MAX - ( double ) radius + 0.5 );
            j = ( index_t ) ( 2. * ( double ) radius * ( double ) rand( ) / ( double ) RAND_MAX - ( double ) radius + 0.5 );
        } else {
            i = ( index_t ) ( ( double ) radius * ( double ) rand( ) / ( double ) RAND_MAX + 0.5 );
            j = ( index_t ) ( ( double ) radius * ( double ) rand( ) / ( double ) RAND_MAX + 0.5 );
        }
        double rad;
        index_t ai = i >= 0 ? i : -i;
        index_t aj = j >= 0 ? j : -j;
        if ( norm >= 1000 ) {
            rad = ai > aj ? ai : aj;
        } else if ( norm == 1 ) {
            rad = ai + aj;
        } else {
            rad = pow ( pow ( ( double ) i, ( double ) norm ) + pow ( ( double ) j, ( double ) norm ), 1.0 / ( double ) norm );
        }
        if ( ( exclude_center && ( rad == 0 ) ) || ( rad > radius ) ) {
            r--;
            continue;
        }
        // see if not already there
        index_t r2;
        for ( r2 = 0 ; r2 < r ; r2++ ) {
            if ( ( i == pt->coords[ r2 ].i ) && ( j == pt->coords[ r2 ].j ) ) {
                break;
            }
        }
        if ( r2 < r ) {
            r--;
            continue;
        }
        pt->coords[ r ].i = i;
        pt->coords[ r ].j = j;
    }
    //
    // symmetrize
    //
    return pt;
}

/*---------------------------------------------------------------------------------------*/
patch_template_t * read_template_old ( const char * fname ) {
    double * tpldat  = 0;
    index_t nrows = 0;
    index_t ncols = 0;
    patch_template_t * pt;
    read_ascii_matrix ( fname, &nrows, &ncols, &tpldat );
    printf ( "TEMPLATE (k=%ld):\n", ncols );
    print_ascii_matrix ( nrows, ncols, tpldat );
    pt = alloc_patch_template ( ncols );
    for ( index_t r = 0 ; r < ncols ; ++r ) {
        pt->coords[ r ].i = tpldat[ r ];
        pt->coords[ r ].j = tpldat[ ncols + r ];
    }
    free ( tpldat );
    return pt;
}

/*---------------------------------------------------------------------------------------*/

patch_template_t * read_one_template ( FILE* fh ) {
    //
    // read template size
    //
    int k = 0;
    if (fscanf(fh," %d ",&k) < 1) {
        fprintf(stderr,"Invalid template file \n");
        return NULL;
    }
    printf("template has %d positions.",k);
    if (k <= 0) {
        fprintf(stderr,"Invalid template file \n");
        return NULL;
    }
    
    patch_template_t * pt = alloc_patch_template(k);

    int i,j,r;
    for (r = 0; fscanf(fh, " %d %d ",&i,&j) == 2 ; ++r) {
        pt->coords[r].i = i;
        pt->coords[r].j = j;
    }
    if (r < k) { // short of coordinates
        fprintf(stderr,"Invalid template file \n");
        free_patch_template(pt);
        return NULL;
    }
    return pt;
}

/*---------------------------------------------------------------------------------------*/

patch_template_t * read_template ( const char * fname ) {
    FILE* fh = fopen(fname,"r");
    if (!fh) {
        fprintf(stderr,"Cannot open template file %s\n",fname);
        return NULL;
    }
    patch_template_t* pt = read_one_template(fh);
    fclose(fh);
    return pt;

}

/*---------------------------------------------------------------------------------------*/

void print_template ( const patch_template_t * ptpl ) {
    index_t min_i = 10000, min_j = 10000, max_i = -10000, max_j = -10000;
    index_t k, r, l;
    for ( k = 0 ; k < ptpl->k ; k++ ) {
        if ( ptpl->coords[ k ].i < min_i ) min_i = ptpl->coords[ k ].i;
        if ( ptpl->coords[ k ].i > max_i ) max_i = ptpl->coords[ k ].i;
        if ( ptpl->coords[ k ].j < min_j ) min_j = ptpl->coords[ k ].j;
        if ( ptpl->coords[ k ].j > max_j ) max_j = ptpl->coords[ k ].j;
    }
    index_t a = max_i - min_i;
    index_t b = max_j - min_j;
    printf ( "    |" );
    for ( r = 0 ; r <= b ; r++ ) {
        printf ( " %2ld ", r + min_j );
    }
    printf ( "\n----+" );
    for ( r = 0 ; r <= b ; r++ ) {
        printf ( "----" );
    }
    putchar ( '\n' );
    for ( k = 0 ; k <= a ; k++ ) {
        printf ( " %2ld |", min_i + k );
        for ( r = 0 ; r <= b ; r++ ) {
            for ( l = 0 ; l < ptpl->k ; l++ ) {
                if ( ( ptpl->coords[ l ].i == ( min_i + k ) ) && ( ptpl->coords[ l ].j == ( min_j + r ) ) ) {
                    printf ( " %2ld ", l );
                    break;
                }
            }
            if ( l == ptpl->k )
                printf ( "    " );
        }
        putchar ( '\n' );
    }

}

/*---------------------------------------------------------------------------------------*/

void dump_template ( const patch_template_t * ptpl, FILE * ft ) {
    index_t k;
    fprintf( ft, "%ld\n",k);
    for ( k = 0 ; k < ptpl->k ; k++ ) {
        fprintf ( ft, "%3ld %3ld\n", ptpl->coords[ k ].i, ptpl->coords[ k ].j );
    }
}

/*---------------------------------------------------------------------------------------*/

int save_template ( const char* fname, const patch_template_t * ptpl ) {
    FILE* fh = fopen(fname,"w");
    if (!fh) {
        fprintf(stderr,"Error writing template to file %s\n",fname);
        return -1;
    }
    dump_template(ptpl, fh);
    fclose(fh);
}

/*---------------------------------------------------------------------------------------*/

patch_template_t * symmetrize_template ( const patch_template_t * in ) {
    const index_t k = in->k;
    patch_template_t * out = alloc_patch_template ( 4 * k );
    index_t r, sk = k;
    for ( r = 0 ; r < k ; r++ ) {
        const index_t i = in->coords[ r ].i;
        const index_t j = in->coords[ r ].j;
        out->coords[ r ].i = i;
        out->coords[ r ].j = j;
        if ( ( i != 0 ) && ( j != 0 ) ) { // i != 0, j != 0
            out->coords[ sk ].i   = -i;
            out->coords[ sk++ ].j =  j;
            out->coords[ sk ].i   =  i;
            out->coords[ sk++ ].j = -j;
        }
        out->coords[ sk ].i = -i;
        out->coords[ sk++ ].j = -j;
    }
    out->k = sk;
    return out;
}

/*---------------------------------------------------------------------------------------*/

patch_template_t * generate_ball_template ( index_t radius, index_t norm, index_t exclude_center ) {
    assert ( radius > 0 );
    patch_template_t * pt = alloc_patch_template ( ( 2 * radius + 1 ) * ( 2 * radius + 1 ) ); // largest possible context
    index_t i, j;
    index_t k = 0;
    for ( i = -radius ; i <= radius ; i++ ) {
        for ( j = -radius ; j <= radius ; j++ ) {
            if ( exclude_center && ( i == 0 ) && ( j == 0 ) ) continue;
            double rad;
            index_t ai = i >= 0 ? i : -i;
            index_t aj = j >= 0 ? j : -j;
            if ( norm >= 1000 ) {
                rad = ai > aj ? ai : aj;
            } else if ( norm == 1 ) {
                rad = ai + aj;
            } else {
                rad = pow ( pow ( ( double ) ai, ( double ) norm ) + pow ( ( double ) aj, ( double ) norm ), 1.0 / ( double ) norm );
            }
            if ( rad <= radius ) {
                pt->coords[ k ].i = i;
                pt->coords[ k++ ].j = j;
            }
        }
    }
    pt->k = k;
    return pt;
}

/*---------------------------------------------------------------------------------------*/
#if 0
static index_t compare_coords_r ( const void * pa, const void * pb, void * pnorm ) {
    const coord_t * a = ( coord_t * ) pa;
    const coord_t * b = ( coord_t * ) pb;
    const index_t norm = *( ( index_t * ) pnorm );
    //
    // first compare norms: smaller goes first
    //
    const double na =
        pow ( fabs ( ( double ) a->i ), ( double ) norm ) +
        pow ( fabs ( ( double ) a->j ), ( double ) norm );
    const double nb =
        pow ( fabs ( ( double ) b->i ), ( double ) norm ) +
        pow ( fabs ( ( double ) b->j ), ( double ) norm );
    if ( na < nb ) {
        return -1;
    } else if ( na > nb ) {
        return 1;
    } else {
        // compare angles
        const double aa = atan2 ( ( double ) a->j, ( double ) a->i );
        const double ab = atan2 ( ( double ) b->j, ( double ) b->i );
        return ( aa < ab ) ? -1 : ( ( aa > ab ) ? 1 : 0 );
    }
}
#endif

/*---------------------------------------------------------------------------------------*/

static int compare_coords ( const void * pa, const void * pb ) {
    const coord_t * a = ( coord_t * ) pa;
    const coord_t * b = ( coord_t * ) pb;
    const index_t norm = 2;
    //
    // first compare norms: smaller goes first
    //
    const double na =
        pow ( fabs ( ( double ) a->i ), ( double ) norm ) +
        pow ( fabs ( ( double ) a->j ), ( double ) norm );
    const double nb =
        pow ( fabs ( ( double ) b->i ), ( double ) norm ) +
        pow ( fabs ( ( double ) b->j ), ( double ) norm );
    if ( na < nb ) {
        return -1;
    } else if ( na > nb ) {
        return 1;
    } else {
        // compare angles
        const double aa = atan2 ( ( double ) a->j, ( double ) a->i );
        const double ab = atan2 ( ( double ) b->j, ( double ) b->i );
        return ( aa < ab ) ? -1 : ( ( aa > ab ) ? 1 : 0 );
    }
}

/*---------------------------------------------------------------------------------------*/

patch_template_t * sort_template ( patch_template_t * orig, index_t in_place ) {
    patch_template_t * out;
    if ( in_place ) {
        out = orig;
    } else {
        out = alloc_patch_template ( orig->k );
        memcpy ( out->coords, orig->coords, orig->k * sizeof( coord_t ) );
    }
    qsort ( out->coords, out->k, sizeof( coord_t ), compare_coords );
    return out;
}

/*---------------------------------------------------------------------------------------*/

patch_template_t * dilate_template ( patch_template_t* in, index_t scale, index_t in_place) {
    patch_template_t* out;
    if (in_place) {
        out = in;
    } else {
        out = alloc_patch_template(in->k);
    }
    for ( index_t r = 0 ; r < in->k ; r++ ) { 
        out->coords[r].i *= scale;
        out->coords[r].j *= scale;
    }
    return out;
}
