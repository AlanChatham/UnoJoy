/*
 * dfu-programmer
 *
 * intel_hex.c
 *
 * This reads in a .hex file (Intel format), creates an array representing
 * memory, populates the array with the data from the .hex file, and
 * returns the array.
 *
 * This implementation is based completely on San Bergmans description
 * of this file format, last updated on 23 August, 2005.
 *
 * http://www.sbprojects.com
 * In the "Knowledge Base" section.
 *
 * $Id: intel_hex.c 80 2009-01-19 07:31:33Z schmidtw $
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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "intel_hex.h"

struct intel_record {
    unsigned int count;
    unsigned int type;
    unsigned int checksum;
    unsigned int address;
    char data[256];
};


/*
 *  This walks over the record and ensures that the checksum is
 *  correct for the record.
 *
 *  returns 0 if checksum validates, anything else on error
 */
static int intel_validate_checksum( struct intel_record *record )
{
    int i = 0;
    int checksum = 0;

    checksum = record->count + record->type + record->checksum +
                        (record->address >> 8) + (0xff & record->address);

    for( i = 0; i < record->count; i++ ) {
        checksum += record->data[i];
    }

    return (0xff & checksum);
}


static int intel_validate_line( struct intel_record *record )
{
    /* Validate the checksum */
    if( 0 != intel_validate_checksum(record) ) {
        return -1;
    }

    /* Validate the type */
    switch( record->type ) {
        /* Intel 1 format, for up to 64K length (types 0, 1) */
        case 0:                             /* data record */
            /* Nothing to do. */
            break;

        case 1:                             /* EOF record */
            if( 0 != record->count )
                return -2;
            break;

        /* Intel 2 format, when 20 bit addresses are needed (types 2, 3, 4) */
        case 2:                             /* extended address record */
            if(    (0 != record->address)
                || (2 != record->count)
                || (record->data[1] != (0xf8 & record->data[1])) )
            {
                return -3;
            }
            break;

        case 3:                             /* start address record */
            /* just ignore these records (could verify addr == 0) */
            return -8;

        case 4:                             /* extended linear address record */
            if( (0 != record->address) || (2 != record->count) )
                return -4;
            break;

        case 5:                             /* start linear address record */
            if( (0 != record->address) || (4 != record->count) ) {
                return -6;
            }
            break;

        default:
            fprintf( stderr, "Unsupported type. %d\n", record->type );
            /* Type 5 and other types are unsupported. */
            return -5;
    }

    return 0;
}


static void intel_process_address( struct intel_record *record )
{
    switch( record->type ) {
        case 2:
            /* 0x1238 -> 0x00012380 */
            record->address = (record->data[0] << 8) | record->data[1];
            record->address *= 16;
            break;

        case 4:
            /* 0x1234 -> 0x12340000 */
            record->address = (record->data[0] << 8) | record->data[1];
            record->address <<= 16;
            break;

        case 5:
            /* 0x12345678 -> 0x12345678 */
            record->address = ((0xff & record->data[0]) << 24) |
                              ((0xff & record->data[1]) << 16) |
                              ((0xff & record->data[2]) <<  8) |
                               (0xff & record->data[3]);
            break;
    }
}


static int intel_read_data( FILE *fp, struct intel_record *record )
{
    int i;
    int c;
    int status;
    char buffer[10];
    int addr_upper = 0;
    int addr_lower = 0;

    /* read in the ':bbaaaarr'
     *   bb - byte count
     * aaaa - the address in memory
     *   rr - record type
     */
    if( NULL == fgets(buffer, 10, fp) ) return -1;
    status = sscanf( buffer, ":%02x%02x%02x%02x", &(record->count),
                     &addr_upper, &addr_lower, &(record->type) );
    if( 4 != status ) return -2;

    record->address = addr_upper << 8 | addr_lower;

    /* Read the data */
    for( i = 0; i < record->count; i++ ) {
        int data = 0;

        if( NULL == fgets(buffer, 3, fp) ) return -3;
        if( 1 != sscanf(buffer, "%02x", &data) ) return -4;

        record->data[i] = 0xff & data;
    }

    /* Read the checksum */
    if( NULL == fgets(buffer, 3, fp) ) return -5;
    if( 1 != sscanf(buffer, "%02x", &(record->checksum)) ) return -6;

    /* Chomp the [\r]\n */
    c = fgetc( fp );
    if( '\r' == c ) {
        c = fgetc( fp );
    }
    if( '\n' != c ) {
        return -7;
    }

    return 0;
}


static int intel_parse_line( FILE *fp, struct intel_record *record )
{
    if( 0 != intel_read_data(fp, record) )
        return -1;

    switch( intel_validate_line(record) ) {
        case 0:     /* data, extended address, etc */
            intel_process_address( record );
            break;

        case -8:    /* start address (ignore) */
            break;

        default:
            return -1;
    }

    return 0;
}

int16_t *intel_hex_to_buffer( char *filename, int max_size, int *usage )
{
    int16_t *memory = NULL;
    FILE *fp = NULL;
    int failure = 1;
    struct intel_record record;
    unsigned int address = 0;
    unsigned int address_offset = 0;
    int i = 0;

    if( (NULL == filename) || (0 >= max_size)  ) {
        fprintf( stderr, "Invalid filename or max_size.\n" );
        goto error;
    }

    if( 0 == strcmp("STDIN",filename) ) {
        fp = stdin;
    } else {
        fp = fopen( filename, "r" );
        if( NULL == fp ) {
            fprintf( stderr, "Error opening the file.\n" );
            goto error;
        }
    }

    memory = (int16_t *) malloc( max_size * sizeof(int16_t) );
    if( NULL == memory ) {
        fprintf( stderr, "Error getting the needed memory.\n" );
        goto error;
    }

    for( i = 0; i < max_size; i++ ) {
        memory[i] = -1;
    }

    *usage = 0;
    do {
        if( 0 != intel_parse_line(fp, &record) ) {
            fprintf( stderr, "Error parsing the line.\n" );
            goto error;
        }

        switch( record.type ) {
            case 0:
                address = address_offset + record.address;
                for( i = 0; i < record.count; i++ ) {
                    if( address >= max_size ) {
                        fprintf( stderr, "Address error.\n" );
                        goto error;
                    }

                    memory[address++] = 0xff & record.data[i];
                    (*usage)++;
                }
                break;

            case 2:
            case 4:
            case 5:
                /* Note: AVR32 "User Page" data will bother this algorithm because
                 * that starts at 0x00800000 this will be out of range and cause
                 * errors until "User Page" programming is implemented.  See section
                 * "18.4.3 User page" in the AT32UC3A datasheet for more details. */

                /* Note: In AVR32 memory map, FLASH starts at 0x80000000, but the
                 * ISP places this memory at 0.  The hex file will use 0x8..., so
                 * mask off that bit. */
                address_offset = (0x7fffffff & record.address);
                break;
        }

    } while( (1 != record.type) );

    failure = 0;

error:

    if( NULL != fp ) {
        fclose( fp );
        fp = NULL;
    }

    if( (NULL != memory) && (0 != failure) ) {
        free( memory );
        memory = NULL;
    }

    return memory;
}
