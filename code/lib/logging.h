/*
 * Copyright (c) 2019 Ignacio Francisco Ramírez Paulino and Ignacio Hounie
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero
 * General Public License for more details.
 * You should have received a copy of the GNU Affero General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
/**
 * \file log.h
 * \brief Bare bones logger
 */
#ifndef LOG_H
#define LOG_H
#include <stdarg.h>
#include <stdio.h>

#define LOG_NONE    100000
#define LOG_ERROR   100
#define LOG_WARNING 90
#define LOG_QUIET   80
#define LOG_INFO    70
#define LOG_DEBUG   60
#define LOG_ALL     0

void set_log_level ( int level );
void set_log_stream ( FILE* stream );

void warn ( const char * format, ... );
void info ( const char * format, ... );
void error ( const char * format, ... );
void debug ( const char * format, ... );

char level_warn ( void );
char level_info ( void );
char level_error ( void );
char level_debug ( void );

#endif
