/*
 * dfu-programmer
 *
 * $Id: commands.c 95 2010-05-02 07:32:39Z schmidtw $
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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "dfu-bool.h"
#include "config.h"
#include "commands.h"
#include "arguments.h"
#include "intel_hex.h"
#include "atmel.h"
#include "util.h"

#define COMMAND_DEBUG_THRESHOLD 40

#define DEBUG(...)  dfu_debug( __FILE__, __FUNCTION__, __LINE__, \
                               COMMAND_DEBUG_THRESHOLD, __VA_ARGS__ )


static int32_t execute_erase( dfu_device_t *device,
                              struct programmer_arguments *args )
{
    int32_t result = 0;

    DEBUG( "erase %d bytes\n",
           (args->flash_address_top - args->flash_address_bottom) );

    result = atmel_erase_flash( device, ATMEL_ERASE_ALL );
    if( 0 != result ) {
        return result;
    }

    return atmel_blank_check( device, args->flash_address_bottom,
                              args->flash_address_top );
}


static int32_t execute_flash_eeprom( dfu_device_t *device,
                                     struct programmer_arguments *args )
{
    int32_t result;
    int32_t i;
    int32_t retval;
    int32_t usage;
    uint8_t *buffer = NULL;
    int16_t *hex_data = NULL;

    retval = -1;

    buffer = (uint8_t *) malloc( args->eeprom_memory_size );
    if( NULL == buffer ) {
        fprintf( stderr, "Request for %d bytes of memory failed.\n",
                 args->eeprom_memory_size );
        goto error;
    }
    memset( buffer, 0, args->eeprom_memory_size );

    hex_data = intel_hex_to_buffer( args->com_flash_data.file,
                                    args->eeprom_memory_size, &usage );
    if( NULL == hex_data ) {
        DEBUG( "Something went wrong with creating the memory image.\n" );
        fprintf( stderr,
                 "Something went wrong with creating the memory image.\n" );
        goto error;
    }

    result = atmel_flash( device, hex_data, 0, args->eeprom_memory_size,
                          args->eeprom_page_size, true );

    if( result < 0 ) {
        DEBUG( "Error while flashing. (%d)\n", result );
        fprintf( stderr, "Error while flashing.\n" );
        goto error;
    }

    if( 0 == args->com_flash_data.suppress_validation ) {
        fprintf( stderr, "Validating...\n" );

        result = atmel_read_flash( device, 0, args->eeprom_memory_size,
                                   buffer, args->eeprom_memory_size, true, false );

        if( args->eeprom_memory_size != result ) {
            DEBUG( "Error while validating.\n" );
            fprintf( stderr, "Error while validating.\n" );
            goto error;
        }

        for( i = 0; i < result; i++ ) {
            if( (0 <= hex_data[i]) && (hex_data[i] < UINT8_MAX) ) {
                /* Memory should have been programmed in this location. */
                if( ((uint8_t) hex_data[i]) != buffer[i] ) {
                    DEBUG( "Image did not validate at location: %d (%02x != %02x)\n", i,
                           (0xff & hex_data[i]), (0xff & buffer[i]) );
                    fprintf( stderr, "Image did not validate.\n" );
                    goto error;
                }
            }
        }
    }

    if( 0 == args->quiet ) {
        fprintf( stderr, "%d bytes used (%.02f%%)\n", usage,
                         ((float)(usage*100)/(float)(args->eeprom_memory_size)) );
    }

    retval = 0;

error:
    if( NULL != buffer ) {
        free( buffer );
        buffer = NULL;
    }

    if( NULL != hex_data ) {
        free( hex_data );
        hex_data = NULL;
    }

    return retval;
}

static int32_t execute_flash_user_page( dfu_device_t *device,
                                        struct programmer_arguments *args )
{
    int32_t result;
    int32_t i;
    int32_t retval;
    int32_t usage;
    uint8_t *buffer = NULL;
    int16_t *hex_data = NULL;

    retval = -1;

    buffer = (uint8_t *) malloc( args->flash_page_size );
    if( NULL == buffer ) {
        fprintf( stderr, "Request for %d bytes of memory failed.\n",
                 args->flash_page_size );
        goto error;
    }
    memset( buffer, 0, args->flash_page_size );

    hex_data = intel_hex_to_buffer( args->com_flash_data.file,
                                    args->flash_page_size, &usage );
    if( NULL == hex_data ) {
        DEBUG( "Something went wrong with creating the memory image.\n" );
        fprintf( stderr,
                 "Something went wrong with creating the memory image.\n" );
        goto error;
    }

    result = atmel_user( device, hex_data, args->flash_page_size );

    if( result < 0 ) {
        DEBUG( "Error while flashing. (%d)\n", result );
        fprintf( stderr, "Error while flashing.\n" );
        goto error;
    }

    if( 0 == args->com_flash_data.suppress_validation ) {
        fprintf( stderr, "Validating...\n" );

        result = atmel_read_flash( device, 0, args->flash_page_size,
                                   buffer, args->flash_page_size, false, true );

        if( args->flash_page_size != result ) {
            DEBUG( "Error while validating.\n" );
            fprintf( stderr, "Error while validating.\n" );
            goto error;
        }

        for( i = 0; i < result; i++ ) {
            if( (0 <= hex_data[i]) && (hex_data[i] < UINT8_MAX) ) {
                /* Memory should have been programmed in this location. */
                if( ((uint8_t) hex_data[i]) != buffer[i] ) {
                    DEBUG( "Image did not validate at location: %d (%02x != %02x)\n", i,
                           (0xff & hex_data[i]), (0xff & buffer[i]) );
                    fprintf( stderr, "Image did not validate.\n" );
                    goto error;
                }
            }
        }
    }

    if( 0 == args->quiet ) {
        fprintf( stderr, "%d bytes used (%.02f%%)\n", usage,
                         ((float)(usage*100)/(float)(args->flash_page_size)) );
    }

    retval = 0;

error:
    if( NULL != buffer ) {
        free( buffer );
        buffer = NULL;
    }

    if( NULL != hex_data ) {
        free( hex_data );
        hex_data = NULL;
    }

    return retval;

}

static int32_t execute_flash_normal( dfu_device_t *device,
                                     struct programmer_arguments *args )
{
    int16_t *hex_data = NULL;
    int32_t  usage = 0;
    int32_t  retval = -1;
    int32_t  result = 0;
    uint8_t *buffer = NULL;
    int32_t  i,j;
    uint32_t memory_size;
    uint32_t adjusted_flash_top_address;

    /* Why +1? Because the flash_address_top location is inclusive, as
     * apposed to most times when sizes are specified by length, etc.
     * and they are exclusive. */
    adjusted_flash_top_address = args->flash_address_top + 1;

    memory_size = adjusted_flash_top_address - args->flash_address_bottom;

    buffer = (uint8_t *) malloc( memory_size );

    if( NULL == buffer ) {
        fprintf( stderr, "Request for %d bytes of memory failed.\n",
                 memory_size );
        goto error;
    }

    memset( buffer, 0, memory_size );

    hex_data = intel_hex_to_buffer( args->com_flash_data.file,
                                    args->memory_address_top + 1, &usage );
    if( NULL == hex_data ) {
        DEBUG( "Something went wrong with creating the memory image.\n" );
        fprintf( stderr,
                 "Something went wrong with creating the memory image.\n" );
        goto error;
    }

    for( i = args->bootloader_bottom; i <= args->bootloader_top; i++) {
        if( -1 != hex_data[i] ) {
            if( true == args->suppressbootloader ) {
                //If we're ignoring the bootloader, don't write to it
                hex_data[i] = -1;
            } else {
                fprintf( stderr, "Bootloader and code overlap.\n" );
                fprintf( stderr, "Use --suppress-bootloader-mem to ignore\n" );
                goto error;
            }
        }
    }

    DEBUG( "write %d/%d bytes\n", usage, memory_size );

    result = atmel_flash( device, hex_data, args->flash_address_bottom,
                          adjusted_flash_top_address, args->flash_page_size, false );

    if( result < 0 ) {
        DEBUG( "Error while flashing. (%d)\n", result );
        fprintf( stderr, "Error while flashing.\n" );
        goto error;
    }

    if( 0 == args->com_flash_data.suppress_validation ) {
        fprintf( stderr, "Validating...\n" );

        result = atmel_read_flash( device, args->flash_address_bottom,
                                   adjusted_flash_top_address, buffer,
                                   memory_size, false, false );

        if( memory_size != result ) {
            DEBUG( "Error while validating.\n" );
            fprintf( stderr, "Error while validating.\n" );
            goto error;
        }

        for( i = 0, j = args->flash_address_bottom; i < result; i++, j++ ) {
            if( (0 <= hex_data[j]) && (hex_data[j] < UINT8_MAX) ) {
                /* Memory should have been programmed in this location. */
                if( ((uint8_t) hex_data[j]) != buffer[i] ) {
                    DEBUG( "Image did not validate at location: %d (%02x != %02x)\n", i,
                           (0xff & hex_data[j]), (0xff & buffer[i]) );
                    fprintf( stderr, "Image did not validate.\n" );
                    goto error;
                }
            }
        }
    }

    if( 0 == args->quiet ) {
        fprintf( stderr, "%d bytes used (%.02f%%)\n", usage,
                         ((float)(usage*100)/(float)
                         (adjusted_flash_top_address - args->flash_address_bottom)) );
    }

    retval = 0;

error:
    if( NULL != buffer ) {
        free( buffer );
        buffer = NULL;
    }

    if( NULL != hex_data ) {
        free( hex_data );
        hex_data = NULL;
    }

    return retval;

    return 0;
}

static int32_t execute_getfuse( dfu_device_t *device,
                            struct programmer_arguments *args )
{
    atmel_avr32_fuses_t info;
    char *message = NULL;
    int32_t value = 0;
    int32_t status;

    status = atmel_read_fuses( device, &info );

    if( 0 != status ) {
        DEBUG( "Error reading %s config information.\n",
               args->device_type_string );
        fprintf( stderr, "Error reading %s config information.\n",
                         args->device_type_string );
        return status;
    }

    switch( args->com_getfuse_data.name ) {
        case get_lock:
            value = info.lock;
            message = "Locked regions";
            break;
        case get_epfl:
            value = info.epfl;
            message = "External Privileged Fetch Lock";
            break;
        case get_bootprot:
            value = info.bootprot;
            message = "Bootloader protected area";
            break;
        case get_bodlevel:
            value = info.bodlevel;
            message = "Brown-out detector trigger level";
            break;
        case get_bodhyst:
            value = info.bodhyst;
            message = "BOD Hysteresis enable";
            break;
        case get_boden:
            value = info.boden;
            message = "BOD Enable";
            break;
        case get_isp_bod_en:
            value = info.isp_bod_en;
            message = "ISP BOD enable";
            break;
        case get_isp_io_cond_en:
            value = info.isp_io_cond_en;
            message = "ISP IO condition enable";
            break;
        case get_isp_force:
            value = info.isp_force;
            message = "ISP Force";
            break;
    }
    fprintf( stdout, "%s%s0x%02x (%d)\n", 
             ((0 == args->quiet) ? message : ""),
             ((0 == args->quiet) ? ": " : ""),
             value, value );
    return 0;
}


static int32_t execute_get( dfu_device_t *device,
                            struct programmer_arguments *args )
{
    atmel_device_info_t info;
    char *message = NULL;
    int16_t value = 0;
    int32_t status;
    int32_t controller_error = 0;

    status = atmel_read_config( device, &info );

    if( 0 != status ) {
        DEBUG( "Error reading %s config information.\n",
               args->device_type_string );
        fprintf( stderr, "Error reading %s config information.\n",
                         args->device_type_string );
        return status;
    }

    switch( args->com_get_data.name ) {
        case get_bootloader:
            value = info.bootloaderVersion;
            message = "Bootloader Version";
            break;
        case get_ID1:
            value = info.bootID1;
            message = "Device boot ID 1";
            break;
        case get_ID2:
            value = info.bootID2;
            message = "Device boot ID 2";
            break;
        case get_BSB:
            value = info.bsb;
            message = "Boot Status Byte";
            if( adc_8051 != args->device_type ) {
                controller_error = 1;
            }
            break;
        case get_SBV:
            value = info.sbv;
            message = "Software Boot Vector";
            if( adc_8051 != args->device_type ) {
                controller_error = 1;
            }
            break;
        case get_SSB:
            value = info.ssb;
            message = "Software Security Byte";
            if( adc_8051 != args->device_type ) {
                controller_error = 1;
            }
            break;
        case get_EB:
            value = info.eb;
            message = "Extra Byte";
            if( adc_8051 != args->device_type ) {
                controller_error = 1;
            }
            break;
        case get_manufacturer:
            value = info.manufacturerCode;
            message = "Manufacturer Code";
            break;
        case get_family:
            value = info.familyCode;
            message = "Family Code";
            break;
        case get_product_name:
            value = info.productName;
            message = "Product Name";
            break;
        case get_product_rev:
            value = info.productRevision;
            message = "Product Revision";
            break;
        case get_HSB:
            value = info.hsb;
            message = "Hardware Security Byte";
            if( adc_8051 != args->device_type ) {
                controller_error = 1;
            }
            break;
    }

    if( 0 != controller_error ) {
        DEBUG( "%s requires 8051 based controller\n", message );
        fprintf( stderr, "%s requires 8051 based controller\n",
                         message );
        return -1;
    }

    if( value < 0 ) {
        fprintf( stderr, "The requested device info is unavailable.\n" );
        return -2;
    }

    fprintf( stdout, "%s%s0x%02x (%d)\n", 
             ((0 == args->quiet) ? message : ""),
             ((0 == args->quiet) ? ": " : ""),
             value, value );
    return 0;
}


static int32_t execute_dump_normal( dfu_device_t *device,
                                    struct programmer_arguments *args )
{
    int32_t i = 0;
    uint8_t *buffer = NULL;
    size_t memory_size;
    size_t adjusted_flash_top_address;

    /* Why +1? Because the flash_address_top location is inclusive, as
     * apposed to most times when sizes are specified by length, etc.
     * and they are exclusive. */
    adjusted_flash_top_address = args->flash_address_top + 1;
    memory_size = adjusted_flash_top_address - args->flash_address_bottom;

    buffer = (uint8_t *) malloc( memory_size );
    if( NULL == buffer ) {
        fprintf( stderr, "Request for %d bytes of memory failed.\n",
                 memory_size );
        goto error;
    }

    DEBUG( "dump %d bytes\n", memory_size );

    if( memory_size != atmel_read_flash(device, args->flash_address_bottom,
                                  adjusted_flash_top_address, buffer,
                                  memory_size, false, false) )
    {
        fprintf( stderr, "Request for %d bytes of memory failed.\n",
                 memory_size );
        return -1;
    }

    if( false == args->bootloader_at_highmem ) {
        for( i = 0; i < args->bootloader_top; i++ ) {
            fprintf( stdout, "%c", 0xff );
        }
    }
    for( i = 0; i < memory_size; i++ ) {
        fprintf( stdout, "%c", buffer[i] );
    }

    fflush( stdout );

error:
    if( NULL != buffer ) {
        free( buffer );
        buffer = NULL;
    }

    return 0;
}

static int32_t execute_dump_eeprom( dfu_device_t *device,
                                    struct programmer_arguments *args )
{
    int32_t i = 0;
    uint8_t *buffer = NULL;
    size_t memory_size;

    memory_size = args->eeprom_memory_size;

    buffer = (uint8_t *) malloc( args->eeprom_memory_size );
    if( NULL == buffer ) {
        fprintf( stderr, "Request for %d bytes of memory failed.\n",
                 memory_size );
        goto error;
    }

    DEBUG( "dump %d bytes\n", memory_size );

    if( memory_size != atmel_read_flash(device, 0,
                                  args->eeprom_memory_size, buffer,
                                  memory_size, true, false) )
    {
        fprintf( stderr, "Request for %d bytes of memory failed.\n",
                 memory_size );
        return -1;
    }

    for( i = 0; i < memory_size; i++ ) {
        fprintf( stdout, "%c", buffer[i] );
    }

    fflush( stdout );

error:
    if( NULL != buffer ) {
        free( buffer );
        buffer = NULL;
    }

    return 0;
}

static int32_t execute_dump_user_page( dfu_device_t *device,
                             struct programmer_arguments *args )
{
    int32_t i = 0;
    uint8_t *buffer = NULL;
    size_t page_size = args->flash_page_size;

    buffer = (uint8_t *) malloc( page_size );
    if( NULL == buffer ) {
        fprintf( stderr, "Request for %d bytes of memory failed.\n",
                 page_size );
        goto error;
    }

    DEBUG( "dump %d bytes\n", page_size );

    if( page_size != atmel_read_flash(device, 0,
                                  page_size, buffer,
                                  page_size, false, true) )
    {
        fprintf( stderr, "Request for %d bytes of memory failed.\n",
                 page_size );
        return -1;
    }

    for( i = 0; i < page_size; i++ ) {
        fprintf( stdout, "%c", buffer[i] );
    }

    fflush( stdout );

error:
    if( NULL != buffer ) {
        free( buffer );
        buffer = NULL;
    }

    return 0;
}

static int32_t execute_setfuse( dfu_device_t *device,
                                  struct programmer_arguments *args )
{
    int32_t value = args->com_setfuse_data.value;
    int32_t name = args->com_setfuse_data.name;

    if( adc_AVR32 != args->device_type ) {
        DEBUG( "target doesn't support fuse set operation.\n" );
        fprintf( stderr, "target doesn't support fuse set operation.\n" );
        return -1;
    }

    if( 0 != atmel_set_fuse(device, name, value) )
    {
        DEBUG( "Fuse set failed.\n" );
        fprintf( stderr, "Fuse set failed.\n" );
        return -1;
    }

    return 0;
}


static int32_t execute_configure( dfu_device_t *device,
                                  struct programmer_arguments *args )
{
    int32_t value = args->com_configure_data.value;
    int32_t name = args->com_configure_data.name;

    if( adc_8051 != args->device_type ) {
        DEBUG( "target doesn't support configure operation.\n" );
        fprintf( stderr, "target doesn't support configure operation.\n" );
        return -1;
    }

    if( (0xff & value) != value ) {
        DEBUG( "Value to configure must be in range 0-255.\n" );
        fprintf( stderr, "Value to configure must be in range 0-255.\n" );
        return -1;
    }

    if( 0 != atmel_set_config(device, name, value) )
    {
        DEBUG( "Configuration set failed.\n" );
        fprintf( stderr, "Configuration set failed.\n" );
        return -1;
    }

    return 0;
}


int32_t execute_command( dfu_device_t *device,
                         struct programmer_arguments *args )
{
    device->type = args->device_type;
    switch( args->command ) {
        case com_erase:
            return execute_erase( device, args );
        case com_flash:
            return execute_flash_normal( device, args );
        case com_eflash:
            return execute_flash_eeprom( device, args );
        case com_user:
            return execute_flash_user_page( device, args );
        case com_reset:
            return atmel_reset( device ); 
        case com_start_app:
            return atmel_start_app( device );
        case com_get:
            return execute_get( device, args );
        case com_getfuse:
            return execute_getfuse( device, args );
        case com_dump:
            return execute_dump_normal( device, args );
        case com_edump:
            return execute_dump_eeprom( device, args );
        case com_udump:
            return execute_dump_user_page( device, args );
        case com_configure:
            return execute_configure( device, args );
        case com_setfuse:
            return execute_setfuse( device, args );
        default:
            fprintf( stderr, "Not supported at this time.\n" );
    }

    return -1;
}
