/*
 * dfu-programmer
 *
 * $Id: atmel.c 25 2006-06-25 00:01:37Z schmidtw $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <stdarg.h>
#include <stdio.h>

#include "util.h"

extern int debug;

void dfu_debug( const char *file, const char *function, const int line,
                const int level, const char *format, ... )
{
    if( level < debug ) {
        va_list va_arg;

        va_start( va_arg, format );
        fprintf( stderr, "%s:%d: ", file, line );
        vfprintf( stderr, format, va_arg );
        va_end( va_arg );
    }
}
