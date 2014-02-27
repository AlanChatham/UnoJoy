#ifndef __DFU_DEVICE_H__
#define __DFU_DEVICE_H__

#if HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdint.h>
#ifdef HAVE_LIBUSB_1_0
#include <libusb.h>
#else
#include <usb.h>
#endif

typedef enum {
    adc_8051,
    adc_AVR,
    adc_AVR32
} atmel_device_class_t;

typedef struct {
#ifdef HAVE_LIBUSB_1_0
    struct libusb_device_handle *handle;
#else
    struct usb_dev_handle *handle;
#endif
    int32_t interface;
    atmel_device_class_t type;
} dfu_device_t;

#endif
