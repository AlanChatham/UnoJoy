/*
 * dfu-programmer
 *
 * $Id: main.c 93 2010-02-23 10:17:40Z schmidtw $
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

#if HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <string.h>
#ifdef HAVE_LIBUSB_1_0
#include <libusb.h>
#else
#include <usb.h>
#endif

#include "config.h"
#include "dfu-device.h"
#include "dfu.h"
#include "atmel.h"
#include "arguments.h"
#include "commands.h"


int debug;
#ifdef HAVE_LIBUSB_1_0
libusb_context *usbcontext;
#endif

int main( int argc, char **argv )
{
    static const char *progname = PACKAGE;
    int retval = 0;
    dfu_device_t dfu_device;
    struct programmer_arguments args;
#ifdef HAVE_LIBUSB_1_0
    struct libusb_device *device = NULL;
#else
    struct usb_device *device = NULL;
#endif

#ifdef HAVE_LIBUSB_1_0
    if (libusb_init(&usbcontext)) {
        fprintf( stderr, "%s: can't init libusb.\n", progname );
    }
#else
    usb_init();
#endif

    memset( &args, 0, sizeof(args) );
    memset( &dfu_device, 0, sizeof(dfu_device) );
    if( 0 != parse_arguments(&args, argc, argv) ) {
        retval = 1;
        goto error;
    }

    if( args.command == com_version ) {
        printf( PACKAGE_STRING "\n" );
        return 0;
    }

    if( debug >= 200 ) {
#ifdef HAVE_LIBUSB_1_0
        libusb_set_debug(usbcontext, debug );
#else
        usb_set_debug( debug );
#endif
    }

    device = dfu_device_init( args.vendor_id, args.chip_id, &dfu_device,
                              args.initial_abort,
                              args.honor_interfaceclass );

    if( NULL == device ) {
        fprintf( stderr, "%s: no device present.\n", progname );
        retval = 1;
        goto error;
    }

    if( 0 != execute_command(&dfu_device, &args) ) {
        /* command issued a specific diagnostic already */
        retval = 1;
        goto error;
    }

    retval = 0;

error:
    if( NULL != dfu_device.handle ) {
        int rv;

#ifdef HAVE_LIBUSB_1_0
        rv = libusb_release_interface( dfu_device.handle, dfu_device.interface );
#else
        rv = usb_release_interface( dfu_device.handle, dfu_device.interface );
#endif
        if( 0 != rv ) {
            fprintf( stderr, "%s: failed to release interface %d.\n",
                             progname, dfu_device.interface );
            retval = 1;
        }
    }

    if( NULL != dfu_device.handle ) {
#ifdef HAVE_LIBUSB_1_0
        libusb_close(dfu_device.handle);
#else
        if( 0 != usb_close(dfu_device.handle) ) {
            fprintf( stderr, "%s: failed to close the handle.\n", progname );
            retval = 1;
        }
#endif
    }
    
#ifdef HAVE_LIBUSB_1_0
    libusb_exit(usbcontext);
#endif

    return retval;
}
