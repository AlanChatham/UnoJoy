/*
 * dfu-programmer
 *
 * $Id: dfu.c 93 2010-02-23 10:17:40Z schmidtw $
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
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#ifdef HAVE_LIBUSB_1_0
#include <libusb.h>
#else
#include <usb.h>
#endif
#include <errno.h>
#include "dfu.h"
#include "util.h"
#include "dfu-bool.h"

/* DFU commands */
#define DFU_DETACH      0
#define DFU_DNLOAD      1
#define DFU_UPLOAD      2
#define DFU_GETSTATUS   3
#define DFU_CLRSTATUS   4
#define DFU_GETSTATE    5
#define DFU_ABORT       6

#define USB_CLASS_APP_SPECIFIC  0xfe
#define DFU_SUBCLASS            0x01

/* Wait for 10 seconds before a timeout since erasing/flashing can take some time. */
#define DFU_TIMEOUT 10000

/* Time (in ms) for the device to wait for the usb reset after being told to detach
 * before the giving up going into dfu mode. */
#define DFU_DETACH_TIMEOUT 1000

#define DFU_DEBUG_THRESHOLD         100
#define DFU_TRACE_THRESHOLD         200
#define DFU_MESSAGE_DEBUG_THRESHOLD 300

#define DEBUG(...)  dfu_debug( __FILE__, __FUNCTION__, __LINE__, \
                               DFU_DEBUG_THRESHOLD, __VA_ARGS__ )
#define TRACE(...)  dfu_debug( __FILE__, __FUNCTION__, __LINE__, \
                               DFU_TRACE_THRESHOLD, __VA_ARGS__ )
#define MSG_DEBUG(...)  dfu_debug( __FILE__, __FUNCTION__, __LINE__, \
                               DFU_MESSAGE_DEBUG_THRESHOLD, __VA_ARGS__ )

static uint16_t transaction = 0;

#ifdef HAVE_LIBUSB_1_0
static int32_t dfu_find_interface( struct libusb_device *device,
                                   const dfu_bool honor_interfaceclass,
                                   const uint8_t bNumConfigurations);
#else
static int32_t dfu_find_interface( const struct usb_device *device,
                                   const dfu_bool honor_interfaceclass );
#endif
static int32_t dfu_make_idle( dfu_device_t *device, const dfu_bool initial_abort );
static int32_t dfu_transfer_out( dfu_device_t *device,
                                 uint8_t request,
                                 const int32_t value,
                                 uint8_t* data,
                                 const size_t length );
static int32_t dfu_transfer_in( dfu_device_t *device,
                                uint8_t request,
                                const int32_t value,
                                uint8_t* data,
                                const size_t length );
static void dfu_msg_response_output( const char *function, const int32_t result );

/* Allocate an N-byte block of memory from the heap.
 *    If N is zero, allocate a 1-byte block.  */
void* rpl_malloc( size_t n )
{
    if( 0 == n ) {
        n = 1;
    }

    return malloc( n );
}


/*
 *  DFU_DETACH Request (DFU Spec 1.0, Section 5.1)
 *
 *  device    - the dfu device to commmunicate with
 *  timeout   - the timeout in ms the USB device should wait for a pending
 *              USB reset before giving up and terminating the operation
 *
 *  returns 0 or < 0 on error
 */
int32_t dfu_detach( dfu_device_t *device, const int32_t timeout )
{
    int32_t result;

    TRACE( "%s( %p, %d )\n", __FUNCTION__, device, timeout );

    if( (NULL == device) || (NULL == device->handle) || (timeout < 0) ) {
        DEBUG( "Invalid parameter\n" );
        return -1;
    }

    result = dfu_transfer_out( device, DFU_DETACH, timeout, NULL, 0 );

    dfu_msg_response_output( __FUNCTION__, result );

    return result;
}


/*
 *  DFU_DNLOAD Request (DFU Spec 1.0, Section 6.1.1)
 *
 *  device    - the dfu device to commmunicate with
 *  length    - the total number of bytes to transfer to the USB
 *              device - must be less than wTransferSize
 *  data      - the data to transfer
 *
 *  returns the number of bytes written or < 0 on error
 */
int32_t dfu_download( dfu_device_t *device, const size_t length, uint8_t* data )
{
    int32_t result;

    TRACE( "%s( %p, %u, %p )\n", __FUNCTION__, device, length, data );

    /* Sanity checks */
    if( (NULL == device) || (NULL == device->handle) ) {
        DEBUG( "Invalid parameter\n" );
        return -1;
    }

    if( (0 != length) && (NULL == data) ) {
        DEBUG( "data was NULL, but length != 0\n" );
        return -2;
    }

    if( (0 == length) && (NULL != data) ) {
        DEBUG( "data was not NULL, but length == 0\n" );
        return -3;
    }


    {
        size_t i;
        for( i = 0; i < length; i++ ) {
            MSG_DEBUG( "Message: m[%u] = 0x%02x\n", i, data[i] );
        }
    }

    result = dfu_transfer_out( device, DFU_DNLOAD, transaction++, data, length );

    dfu_msg_response_output( __FUNCTION__, result );

    return result;
}


/*
 *  DFU_UPLOAD Request (DFU Spec 1.0, Section 6.2)
 *
 *  device    - the dfu device to commmunicate with
 *  length    - the maximum number of bytes to receive from the USB
 *              device - must be less than wTransferSize
 *  data      - the buffer to put the received data in
 *
 *  returns the number of bytes received or < 0 on error
 */
int32_t dfu_upload( dfu_device_t *device, const size_t length, uint8_t* data )
{
    int32_t result;

    TRACE( "%s( %p, %u, %p )\n", __FUNCTION__, device, length, data );

    /* Sanity checks */
    if( (NULL == device) || (NULL == device->handle) ) {
        DEBUG( "Invalid parameter\n" );
        return -1;
    }

    if( (0 == length) || (NULL == data) ) {
        DEBUG( "data was NULL, or length is 0\n" );
        return -2;
    }

    result = dfu_transfer_in( device, DFU_UPLOAD, transaction++, data, length );

    dfu_msg_response_output( __FUNCTION__, result );

    return result;
}


/*
 *  DFU_GETSTATUS Request (DFU Spec 1.0, Section 6.1.2)
 *
 *  device    - the dfu device to commmunicate with
 *  status    - the data structure to be populated with the results
 *
 *  return the 0 if successful or < 0 on an error
 */
int32_t dfu_get_status( dfu_device_t *device, dfu_status_t *status )
{
    uint8_t buffer[6];
    int32_t result;

    TRACE( "%s( %p, %p )\n", __FUNCTION__, device, status );

    if( (NULL == device) || (NULL == device->handle) ) {
        DEBUG( "Invalid parameter\n" );
        return -1;
    }

    /* Initialize the status data structure */
    status->bStatus       = DFU_STATUS_ERROR_UNKNOWN;
    status->bwPollTimeout = 0;
    status->bState        = STATE_DFU_ERROR;
    status->iString       = 0;

    result = dfu_transfer_in( device, DFU_GETSTATUS, 0, buffer, sizeof(buffer) );

    dfu_msg_response_output( __FUNCTION__, result );

    if( 6 == result ) {
        status->bStatus = buffer[0];
        status->bwPollTimeout = ((0xff & buffer[3]) << 16) |
                                ((0xff & buffer[2]) << 8)  |
                                (0xff & buffer[1]);

        status->bState  = buffer[4];
        status->iString = buffer[5];

        DEBUG( "==============================\n" );
        DEBUG( "status->bStatus: %s (0x%02x)\n",
               dfu_status_to_string(status->bStatus), status->bStatus );
        DEBUG( "status->bwPollTimeout: 0x%04x\n", status->bwPollTimeout );
        DEBUG( "status->bState: %s (0x%02x)\n",
               dfu_state_to_string(status->bState), status->bState );
        DEBUG( "status->iString: 0x%02x\n", status->iString );
        DEBUG( "------------------------------\n" );
    } else {
        if( 0 < result ) {
            /* There was an error, we didn't get the entire message. */
            DEBUG( "result: %d\n", result );
            return -2;
        }
    }

    return 0;
}


/*
 *  DFU_CLRSTATUS Request (DFU Spec 1.0, Section 6.1.3)
 *
 *  device    - the dfu device to commmunicate with
 *
 *  return 0 or < 0 on an error
 */
int32_t dfu_clear_status( dfu_device_t *device )
{
    int32_t result;

    TRACE( "%s( %p )\n", __FUNCTION__, device );

    if( (NULL == device) || (NULL == device->handle) ) {
        DEBUG( "Invalid parameter\n" );
        return -1;
    }

    result = dfu_transfer_out( device, DFU_CLRSTATUS, 0, NULL, 0 );

    dfu_msg_response_output( __FUNCTION__, result );

    return result;
}


/*
 *  DFU_GETSTATE Request (DFU Spec 1.0, Section 6.1.5)
 *
 *  device    - the dfu device to commmunicate with
 *
 *  returns the state or < 0 on error
 */
int32_t dfu_get_state( dfu_device_t *device )
{
    int32_t result;
    uint8_t buffer[1];

    TRACE( "%s( %p )\n", __FUNCTION__, device );

    if( (NULL == device) || (NULL == device->handle) ) {
        DEBUG( "Invalid parameter\n" );
        return -1;
    }

    result = dfu_transfer_in( device, DFU_GETSTATE, 0, buffer, sizeof(buffer) );

    dfu_msg_response_output( __FUNCTION__, result );

    /* Return the error if there is one. */
    if( result < 1 ) {
        return result;
    }

    /* Return the state. */
    return buffer[0];
}


/*
 *  DFU_ABORT Request (DFU Spec 1.0, Section 6.1.4)
 *
 *  device    - the dfu device to commmunicate with
 *
 *  returns 0 or < 0 on an error
 */
int32_t dfu_abort( dfu_device_t *device )
{
    int32_t result;

    TRACE( "%s( %p )\n", __FUNCTION__, device );

    if( (NULL == device) || (NULL == device->handle) ) {
        DEBUG( "Invalid parameter\n" );
        return -1;
    }

    result = dfu_transfer_out( device, DFU_ABORT, 0, NULL, 0 );

    dfu_msg_response_output( __FUNCTION__, result );

    return result;
}


/*
 *  dfu_device_init is designed to find one of the usb devices which match
 *  the vendor and product parameters passed in.
 *
 *  vendor  - the vender number of the device to look for
 *  product - the product number of the device to look for
 *  [out] device - the dfu device to commmunicate with
 *
 *  return a pointer to the usb_device if found, or NULL otherwise
 */
#ifdef HAVE_LIBUSB_1_0
struct libusb_device *dfu_device_init( const uint32_t vendor,
                                       const uint32_t product,
                                       dfu_device_t *dfu_device,
                                       const dfu_bool initial_abort,
                                       const dfu_bool honor_interfaceclass )
{
    libusb_device **list;
    size_t i,devicecount;
    extern libusb_context *usbcontext;
    int32_t retries = 4;
    
    TRACE( "%s( %u, %u, %p, %s, %s )\n", __FUNCTION__, vendor, product,
           dfu_device, ((true == initial_abort) ? "true" : "false"),
           ((true == honor_interfaceclass) ? "true" : "false") );

    DEBUG( "%s(%08x, %08x)\n",__FUNCTION__, vendor, product );

retry:
    devicecount = libusb_get_device_list( usbcontext, &list );
    
    for( i = 0; i < devicecount; i++ ) {
        libusb_device *device = list[i];
        struct libusb_device_descriptor descriptor;

        if( libusb_get_device_descriptor(device, &descriptor) ) {
             DEBUG( "Failed in libusb_get_device_descriptor\n" );
             break;
        }
        
        DEBUG( "%2d: 0x%04x, 0x%04x\n", (int) i,
                descriptor.idVendor, descriptor.idProduct );

        if( (vendor  == descriptor.idVendor) &&
            (product == descriptor.idProduct) )
        {
            int32_t tmp;
            /* We found a device that looks like it matches...
             * let's try to find the DFU interface, open the device
             * and claim it. */
            tmp = dfu_find_interface( device, honor_interfaceclass,
                                      descriptor.bNumConfigurations );
            
            if( 0 <= tmp ) {    /* The interface is valid. */
                dfu_device->interface = tmp;

                if( 0 == libusb_open(device, &dfu_device->handle) ) {
                    DEBUG( "opened interface %d...\n", tmp );
                    if( 0 == libusb_set_configuration(dfu_device->handle, 1) ) {
                        DEBUG( "set configuration %d...\n", 1 );
                        if( 0 == libusb_claim_interface(dfu_device->handle, dfu_device->interface) )
                        {
                            DEBUG( "claimed interface %d...\n", dfu_device->interface );

                            switch( dfu_make_idle(dfu_device, initial_abort) )
                            {
                                case 0:
                                    libusb_free_device_list( list, 1 );
                                    return device;

                                case 1:
                                    retries--;
                                    libusb_free_device_list( list, 1 );
                                    goto retry;
                            }

                            DEBUG( "Failed to put the device in dfuIDLE mode.\n" );
                            libusb_release_interface( dfu_device->handle, dfu_device->interface );
                            retries = 4;
                        } else {
                            DEBUG( "Failed to claim the DFU interface.\n" );
                        }
                    } else {
                        DEBUG( "Failed to set configuration.\n" );
                    }

                    libusb_close(dfu_device->handle);
                }
            }
        }
    }

    libusb_free_device_list( list, 1 );
    dfu_device->handle = NULL;
    dfu_device->interface = 0;

    return NULL;
}
#else
struct usb_device *dfu_device_init( const uint32_t vendor,
                                    const uint32_t product,
                                    dfu_device_t *dfu_device,
                                    const dfu_bool initial_abort,
                                    const dfu_bool honor_interfaceclass )
{
    struct usb_bus *usb_bus;
    struct usb_device *device;
    int32_t retries = 4;

    TRACE( "%s( %u, %u, %p, %s, %s )\n", __FUNCTION__, vendor, product,
           dfu_device, ((true == initial_abort) ? "true" : "false"),
           ((true == honor_interfaceclass) ? "true" : "false") );

retry:

    if( 0 < retries ) {
        usb_find_busses();
        usb_find_devices();

        /* Walk the tree and find our device. */
        for( usb_bus = usb_get_busses(); NULL != usb_bus; usb_bus = usb_bus->next ) {
            for( device = usb_bus->devices; NULL != device; device = device->next) {
                if(    (vendor  == device->descriptor.idVendor)
                    && (product == device->descriptor.idProduct) )
                {
                    int32_t tmp;
                    /* We found a device that looks like it matches...
                     * let's try to find the DFU interface, open the device
                     * and claim it. */
                    tmp = dfu_find_interface( device, honor_interfaceclass );
                    if( 0 <= tmp ) {
                        /* The interface is valid. */
                        dfu_device->interface = tmp;
                        dfu_device->handle = usb_open( device );
                        if( NULL != dfu_device->handle ) {
                            if( 0 == usb_set_configuration(dfu_device->handle, 1) ) {
                                if( 0 == usb_claim_interface(dfu_device->handle, dfu_device->interface) ) {
                                    switch( dfu_make_idle(dfu_device, initial_abort) )
                                    {
                                        case 0:
                                            return device;
                                        case 1:
                                            retries--;
                                            goto retry;
                                    }

                                    DEBUG( "Failed to put the device in dfuIDLE mode.\n" );
                                    usb_release_interface( dfu_device->handle, dfu_device->interface );
                                    usb_close( dfu_device->handle );
                                    retries = 4;
                                } else {
                                    DEBUG( "Failed to claim the DFU interface.\n" );
                                    usb_close( dfu_device->handle );
                                }
                            } else {
                                DEBUG( "Failed to set configuration.\n");

                                usb_close( dfu_device->handle );
                            }
                        } else {
                            DEBUG( "Failed to open device.\n" );
                        }
                    } else {
                        DEBUG( "Failed to find the DFU interface.\n" );
                    }
                }
            }
        }
    }

    dfu_device->handle = NULL;
    dfu_device->interface = 0;

    return NULL;
}
#endif


/*
 *  Used to convert the DFU state to a string.
 *
 *  state - the state to convert
 *
 *  returns the state name or "unknown state"
 */
char* dfu_state_to_string( const int32_t state )
{
    char *message = "unknown state";

    switch( state ) {
        case STATE_APP_IDLE:
            message = "appIDLE";
            break;
        case STATE_APP_DETACH:
            message = "appDETACH";
            break;
        case STATE_DFU_IDLE:
            message = "dfuIDLE";
            break;
        case STATE_DFU_DOWNLOAD_SYNC:
            message = "dfuDNLOAD-SYNC";
            break;
        case STATE_DFU_DOWNLOAD_BUSY:
            message = "dfuDNBUSY";
            break;
        case STATE_DFU_DOWNLOAD_IDLE:
            message = "dfuDNLOAD-IDLE";
            break;
        case STATE_DFU_MANIFEST_SYNC:
            message = "dfuMANIFEST-SYNC";
            break;
        case STATE_DFU_MANIFEST:
            message = "dfuMANIFEST";
            break;
        case STATE_DFU_MANIFEST_WAIT_RESET:
            message = "dfuMANIFEST-WAIT-RESET";
            break;
        case STATE_DFU_UPLOAD_IDLE:
            message = "dfuUPLOAD-IDLE";
            break;
        case STATE_DFU_ERROR:
            message = "dfuERROR";
            break;
    }

    return message;
}


/*
 *  Used to convert the DFU status to a string.
 *
 *  status - the status to convert
 *
 *  returns the status name or "unknown status"
 */
char* dfu_status_to_string( const int32_t status )
{
    char *message = "unknown status";

    switch( status ) {
        case DFU_STATUS_OK:
            message = "OK";
            break;
        case DFU_STATUS_ERROR_TARGET:
            message = "errTARGET";
            break;
        case DFU_STATUS_ERROR_FILE:
            message = "errFILE";
            break;
        case DFU_STATUS_ERROR_WRITE:
            message = "errWRITE";
            break;
        case DFU_STATUS_ERROR_ERASE:
            message = "errERASE";
            break;
        case DFU_STATUS_ERROR_CHECK_ERASED:
            message = "errCHECK_ERASED";
            break;
        case DFU_STATUS_ERROR_PROG:
            message = "errPROG";
            break;
        case DFU_STATUS_ERROR_VERIFY:
            message = "errVERIFY";
            break;
        case DFU_STATUS_ERROR_ADDRESS:
            message = "errADDRESS";
            break;
        case DFU_STATUS_ERROR_NOTDONE:
            message = "errNOTDONE";
            break;
        case DFU_STATUS_ERROR_FIRMWARE:
            message = "errFIRMWARE";
            break;
        case DFU_STATUS_ERROR_VENDOR:
            message = "errVENDOR";
            break;
        case DFU_STATUS_ERROR_USBR:
            message = "errUSBR";
            break;
        case DFU_STATUS_ERROR_POR:
            message = "errPOR";
            break;
        case DFU_STATUS_ERROR_UNKNOWN:
            message = "errUNKNOWN";
            break;
        case DFU_STATUS_ERROR_STALLEDPKT:
            message = "errSTALLEDPKT";
            break;

    }

    return message;
}


/*
 *  Used to find the dfu interface for a device if there is one.
 *
 *  device - the device to search
 *  honor_interfaceclass - if the actual interface class information
 *                         should be checked, or ignored (bug in device DFU code)
 *
 *  returns the interface number if found, < 0 otherwise
 */
#ifdef HAVE_LIBUSB_1_0
static int32_t dfu_find_interface( struct libusb_device *device,
                                   const dfu_bool honor_interfaceclass,
                                   const uint8_t bNumConfigurations)
{
    int32_t c,i,s;
    
    TRACE( "%s()\n", __FUNCTION__ );

    /* Loop through all of the configurations */
    for( c = 0; c < bNumConfigurations; c++ ) {
        struct libusb_config_descriptor *config;

        if( libusb_get_config_descriptor(device, c, &config) ) {
            DEBUG( "can't get_config_descriptor: %d\n", c );
            return -1;
        }
        DEBUG( "config %d: maxpower=%d*2 mA\n", c, config->MaxPower );

        /* Loop through all of the interfaces */
        for( i = 0; i < config->bNumInterfaces; i++ ) {
            struct libusb_interface interface;

            interface = config->interface[i];
            DEBUG( "interface %d\n", i );

            /* Loop through all of the settings */
            for( s = 0; s < interface.num_altsetting; s++ ) {
                struct libusb_interface_descriptor setting;

                setting = interface.altsetting[s];
                DEBUG( "setting %d: class:%d, subclass %d, protocol:%d\n", s,
                                setting.bInterfaceClass, setting.bInterfaceSubClass,
                                setting.bInterfaceProtocol );

                if( honor_interfaceclass ) {
                    /* Check if the interface is a DFU interface */
                    if(    (USB_CLASS_APP_SPECIFIC == setting.bInterfaceClass)
                        && (DFU_SUBCLASS == setting.bInterfaceSubClass) )
                    {
                        DEBUG( "Found DFU Interface: %d\n", setting.bInterfaceNumber );
                        return setting.bInterfaceNumber;
                    }
                } else {
                    /* If there is a bug in the DFU firmware, return the first
                     * found interface. */
                    DEBUG( "Found DFU Interface: %d\n", setting.bInterfaceNumber );
                    return setting.bInterfaceNumber;
                }
            }
        }

        libusb_free_config_descriptor( config );
    }

    return -1;
}
#else
static int32_t dfu_find_interface( const struct usb_device *device,
                                   const dfu_bool honor_interfaceclass )
{
    int32_t c, i;
    struct usb_config_descriptor *config;
    struct usb_interface_descriptor *interface;
    
    /* Loop through all of the configurations */
    for( c = 0; c < device->descriptor.bNumConfigurations; c++ ) {
        config = &(device->config[c]);

        /* Loop through all of the interfaces */
        for( i = 0; i < config->interface->num_altsetting; i++) {
            interface = &(config->interface->altsetting[i]);

            if( true == honor_interfaceclass ) {
                /* Check if the interface is a DFU interface */
                if(    (USB_CLASS_APP_SPECIFIC == interface->bInterfaceClass)
                    && (DFU_SUBCLASS == interface->bInterfaceSubClass) )
                {
                    DEBUG( "Found DFU Inteface: %d\n", interface->bInterfaceNumber );
                    return interface->bInterfaceNumber;
                }
            } else {
                /* If there is a bug in the DFU firmware, return the first
                 * found interface. */
                DEBUG( "Found DFU Inteface: %d\n", interface->bInterfaceNumber );
                return interface->bInterfaceNumber;
            }
        }
    }

    return -1;
}
#endif

/*
 *  Gets the device into the dfuIDLE state if possible.
 *
 *  device    - the dfu device to commmunicate with
 *
 *  returns 0 on success, 1 if device was reset, error otherwise
 */
static int32_t dfu_make_idle( dfu_device_t *device,
                              const dfu_bool initial_abort )
{
    dfu_status_t status;
    int32_t retries = 4;

    if( true == initial_abort ) {
        dfu_abort( device );
    }

    while( 0 < retries ) {
        if( 0 != dfu_get_status(device, &status) ) {
            dfu_clear_status( device );
            continue;
        }

        DEBUG( "State: %s (%d)\n", dfu_state_to_string(status.bState), status.bState );

        switch( status.bState ) {
            case STATE_DFU_IDLE:
                if( DFU_STATUS_OK == status.bStatus ) {
                    return 0;
                }

                /* We need the device to have the DFU_STATUS_OK status. */
                dfu_clear_status( device );
                break;

            case STATE_DFU_DOWNLOAD_SYNC:   /* abort -> idle */
            case STATE_DFU_DOWNLOAD_IDLE:   /* abort -> idle */
            case STATE_DFU_MANIFEST_SYNC:   /* abort -> idle */
            case STATE_DFU_UPLOAD_IDLE:     /* abort -> idle */
            case STATE_DFU_DOWNLOAD_BUSY:   /* abort -> error */
            case STATE_DFU_MANIFEST:        /* abort -> error */
                dfu_abort( device );
                break;

            case STATE_DFU_ERROR:
                dfu_clear_status( device );
                break;

            case STATE_APP_IDLE:
                dfu_detach( device, DFU_DETACH_TIMEOUT );
                break;

            case STATE_APP_DETACH:
            case STATE_DFU_MANIFEST_WAIT_RESET:
                DEBUG( "Resetting the device\n" );
#ifdef HAVE_LIBUSB_1_0
                libusb_reset_device( device->handle );
#else
                usb_reset( device->handle );
#endif
                return 1;
        }

        retries--;
    }

    DEBUG( "Not able to transition the device into the dfuIDLE state.\n" );
    return -2;
}


static int32_t dfu_transfer_out( dfu_device_t *device,
                                 uint8_t request,
                                 const int32_t value,
                                 uint8_t* data,
                                 const size_t length )
{
#ifdef HAVE_LIBUSB_1_0
    return libusb_control_transfer( device->handle,
                /* bmRequestType */ LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
                /* bRequest      */ request,
                /* wValue        */ value,
                /* wIndex        */ device->interface,
                /* Data          */ data,
                /* wLength       */ length,
                                    DFU_TIMEOUT );
#else
    return usb_control_msg( device->handle,
                /* bmRequestType */ USB_ENDPOINT_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                /* bRequest      */ request,
                /* wValue        */ value,
                /* wIndex        */ device->interface,
                /* Data          */ (char*) data,
                /* wLength       */ length,
                                    DFU_TIMEOUT );
#endif
}

static int32_t dfu_transfer_in( dfu_device_t *device,
                                uint8_t request,
                                const int32_t value,
                                uint8_t* data,
                                const size_t length )
{
#ifdef HAVE_LIBUSB_1_0
    return libusb_control_transfer( device->handle,
                /* bmRequestType */ LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
                /* bRequest      */ request,
                /* wValue        */ value,
                /* wIndex        */ device->interface,
                /* Data          */ data,
                /* wLength       */ length,
                                    DFU_TIMEOUT );
#else
    return usb_control_msg( device->handle,
                /* bmRequestType */ USB_ENDPOINT_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                /* bRequest      */ request,
                /* wValue        */ value,
                /* wIndex        */ device->interface,
                /* Data          */ (char*) data,
                /* wLength       */ length,
                                    DFU_TIMEOUT );
#endif
}


/*
 *  Used to output the response from our USB request in a human reable
 *  form.
 *
 *  function - the calling function to output on behalf of
 *  result   - the result to interpret
 */
static void dfu_msg_response_output( const char *function, const int32_t result )
{
    char *msg = NULL;

    if( 0 <= result ) {
        msg = "No error.";
    } else {
        switch( result ) {
            case -ENOENT:
                msg = "-ENOENT: URB was canceled by unlink_urb";
                break;
#ifdef EINPROGRESS
            case -EINPROGRESS:
                msg = "-EINPROGRESS: URB still pending, no results yet "
                      "(actually no error until now)";
                break;
#endif
#ifdef EPROTO
            case -EPROTO:
                msg = "-EPROTO: a) Bitstuff error or b) Unknown USB error";
                break;
#endif
            case -EILSEQ:
                msg = "-EILSEQ: CRC mismatch";
                break;
            case -EPIPE:
                msg = "-EPIPE: a) Babble detect or b) Endpoint stalled";
                break;
#ifdef ETIMEDOUT
            case -ETIMEDOUT:
                msg = "-ETIMEDOUT: Transfer timed out, NAK";
                break;
#endif
            case -ENODEV:
                msg = "-ENODEV: Device was removed";
                break;
#ifdef EREMOTEIO
            case -EREMOTEIO:
                msg = "-EREMOTEIO: Short packet detected";
                break;
#endif
            case -EXDEV:
                msg = "-EXDEV: ISO transfer only partially completed look at "
                      "individual frame status for details";
                break;
            case -EINVAL:
                msg = "-EINVAL: ISO madness, if this happens: Log off and go home";
                break;
            default:
                msg = "Unknown error";
                break;
        }

        DEBUG( "%s 0x%08x (%d)\n", msg, result, result );
    }
}
