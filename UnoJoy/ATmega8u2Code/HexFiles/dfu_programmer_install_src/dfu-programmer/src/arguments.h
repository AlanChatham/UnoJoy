/*
 * dfu-programmer
 *
 * $Id: arguments.h 101 2011-01-17 03:09:05Z schmidtw $
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

#ifndef __ARGUMENTS_H__
#define __ARGUMENTS_H__

#include "dfu-bool.h"
#include "dfu-device.h"
#include "atmel.h"

#define DEVICE_TYPE_STRING_MAX_LENGTH   6
/*
 *  atmel_programmer target command
 *
 *  configure {BSB|SBV|SSB|EB|HSB} [--suppress-validation, --quiet, --debug level] value
 *  dump [--quiet, --debug level]
 *  erase [--suppress-validation, --quiet, --debug level]
 *  flash [--suppress-validation, --quiet, --debug level] file
 *  get {bootloader-version|ID1|ID2|BSB|SBV|SSB|EB|manufacturer|family|product-name|product-revision|HSB} [--quiet, --debug level]
 */

extern int debug;

enum targets_enum { tar_at89c51snd1c,
                    tar_at89c51snd2c,
                    tar_at89c5130,
                    tar_at89c5131,
                    tar_at89c5132,
                    tar_at90usb1287,
                    tar_at90usb1286,
                    tar_at90usb1287_4k,
                    tar_at90usb1286_4k,
                    tar_at90usb647,
                    tar_at90usb646,
                    tar_at90usb162,
                    tar_at90usb82,
                    tar_atmega32u6,
                    tar_atmega32u4,
                    tar_atmega32u2,
                    tar_atmega16u4,
                    tar_atmega8u2,
                    tar_atmega16u2,
                    tar_at32uc3b064,
                    tar_at32uc3b164,
                    tar_at32uc3b0128,
                    tar_at32uc3b1128,
                    tar_at32uc3b0256,
                    tar_at32uc3b1256,
                    tar_at32uc3b0256es,
                    tar_at32uc3b1256es,
                    tar_at32uc3b0512,
                    tar_at32uc3b1512,
                    tar_at32uc3a0128,
                    tar_at32uc3a1128,
                    tar_at32uc3a0256,
                    tar_at32uc3a1256,
                    tar_at32uc3a0512,
                    tar_at32uc3a1512,
                    tar_at32uc3a0512es,
                    tar_at32uc3a1512es,
                    tar_at32uc3a364,
                    tar_at32uc3a364s,
                    tar_at32uc3a3128,
                    tar_at32uc3a3128s,
                    tar_at32uc3a3256,
                    tar_at32uc3a3256s,
                    tar_at32uc3c064,
                    tar_at32uc3c0128,
                    tar_at32uc3c0256,
                    tar_at32uc3c0512,
                    tar_at32uc3c164,
                    tar_at32uc3c1128,
                    tar_at32uc3c1256,
                    tar_at32uc3c1512,
                    tar_at32uc3c264,
                    tar_at32uc3c2128,
                    tar_at32uc3c2256,
                    tar_at32uc3c2512,
                    tar_none };

enum commands_enum { com_none, com_erase, com_flash, com_user, com_eflash,
                     com_configure, com_get, com_getfuse, com_dump, com_edump,
                     com_udump, com_setfuse,
                     com_start_app, com_version, com_reset };

enum configure_enum { conf_BSB = ATMEL_SET_CONFIG_BSB,
                      conf_SBV = ATMEL_SET_CONFIG_SBV,
                      conf_SSB = ATMEL_SET_CONFIG_SSB,
                      conf_EB  = ATMEL_SET_CONFIG_EB,
                      conf_HSB = ATMEL_SET_CONFIG_HSB };

enum setfuse_enum { set_lock, set_epfl, set_bootprot, set_bodlevel,
                    set_bodhyst, set_boden, set_isp_bod_en,
                    set_isp_io_cond_en, set_isp_force };

enum get_enum { get_bootloader, get_ID1, get_ID2, get_BSB, get_SBV, get_SSB,
                get_EB, get_manufacturer, get_family, get_product_name,
                get_product_rev, get_HSB };

enum getfuse_enum { get_lock, get_epfl, get_bootprot, get_bodlevel,
                    get_bodhyst, get_boden, get_isp_bod_en,
                    get_isp_io_cond_en, get_isp_force };

struct programmer_arguments {
    /* target-specific inputs */
    enum targets_enum target;
    uint16_t vendor_id;
    uint16_t chip_id;
    atmel_device_class_t device_type;
    char device_type_string[DEVICE_TYPE_STRING_MAX_LENGTH];
    uint32_t memory_address_top;        /* the maximum memory address */
    uint32_t memory_address_bottom;     /* the minimum memory address */
    uint32_t flash_address_top;         /* the maximum flash-able address */
    uint32_t flash_address_bottom;      /* the minimum flash-able address */
    uint32_t bootloader_top;            /* the top of the bootloader code */
    uint32_t bootloader_bottom;         /* the bottom of the bootloader code */
    dfu_bool bootloader_at_highmem;
    size_t flash_page_size;
    dfu_bool initial_abort;
    dfu_bool honor_interfaceclass;
    size_t eeprom_memory_size;
    size_t eeprom_page_size;

    /* command-specific state */
    enum commands_enum command;
    char quiet;
    char suppressbootloader;

    union {
        struct com_configure_struct {
            enum configure_enum name;
            int32_t suppress_validation;
            int32_t value;
        } com_configure_data;

        struct com_setfuse_struct {
            enum setfuse_enum name;
            int32_t value;
        } com_setfuse_data;

        /* No special data needed for 'dump' */

        struct com_erase_struct {
            int32_t suppress_validation;
        } com_erase_data;

        struct com_flash_struct {
            int32_t suppress_validation;
            char original_first_char;
            char *file;
        } com_flash_data;

        struct com_get_struct {
            enum get_enum name;
        } com_get_data;

        struct com_getfuse_struct {
            enum getfuse_enum name;
        } com_getfuse_data;
    };
};

int32_t parse_arguments( struct programmer_arguments *args,
                         const size_t argc,
                         char **argv );
#endif
