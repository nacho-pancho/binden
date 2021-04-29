/*
 * Copyright © 2012, Ignacio Ramírez Paulino <nacho@fing.edu.uy>
 * All rights reserved.
 *
 * This file is part of IRP.
 *
 * IRP is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * IRP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with IRP.  If not, see <http://www.gnu.org/licenses/>.
 */
/**
 * \file irp_pmf.h
 * \ingroup utilities
 * \brief Input/Output functions, mostly for matrixes and vectors.
 * \todo Version 2: read/write incremental matrices (total unknown number of columns in advance): read by chunks
 */
#ifndef ASCMAT_H
#define ASCMAT_H
#include <string.h>
#include <errno.h>
#include <stdio.h>

int scan_ascii_matrix ( const char * fname, unsigned * pnrows, unsigned * pncols );
int read_ascii_matrix ( const char * fname, unsigned * pnrows, unsigned * pncols, double * * matdata );
int read_ascii_data ( const char * fname, const unsigned nrows, const unsigned ncols, double * data );
void print_ascii_matrix ( const unsigned nrows, const unsigned ncols, const double * data );

#endif
