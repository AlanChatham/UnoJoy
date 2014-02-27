/*
 * dfu-programmer
 *
 * $Id: arguments.c 101 2011-01-17 03:09:05Z schmidtw $
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

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dfu-bool.h"
#include "dfu-device.h"
#include "config.h"
#include "arguments.h"

struct option_mapping_structure {
    const char *name;
    int32_t value;
};

struct target_mapping_structure {
    const char *name;
    enum targets_enum value;
    atmel_device_class_t device_type;
    uint16_t chip_id;
    uint16_t vendor_id;
    size_t memory_size;
    size_t bootloader_size;
    dfu_bool bootloader_at_highmem;
    size_t flash_page_size;
    dfu_bool initial_abort;
    dfu_bool honor_interfaceclass;
    size_t eeprom_page_size;
    size_t eeprom_memory_size;
};

/* NOTE FOR: at90usb1287, at90usb1286, at90usb647, at90usb646, at90usb162, at90usb82
 *
 * The actual size of the user-programmable section is limited by the
 * space needed by the bootloader.  The size of the bootloader is set
 * by BOOTSZ0/BOOTSZ1 fuse bits; here we assume the bootloader is 4kb or 8kb.
 * The window used for the bootloader is at the top of the of memory.
 */

/* ----- target specific structures ----------------------------------------- */
static struct target_mapping_structure target_map[] = {
    { "at89c51snd1c",   tar_at89c51snd1c,   adc_8051,  0x2FFF, 0x03eb, 0x10000, 0x1000, true,  128, false, true,  0,   0      },
    { "at89c51snd2c",   tar_at89c51snd2c,   adc_8051,  0x2FFF, 0x03eb, 0x10000, 0x1000, true,  128, false, true,  0,   0      },
    { "at89c5130",      tar_at89c5130,      adc_8051,  0x2FFD, 0x03eb, 0x04000, 0x0000, true,  128, false, true,  128, 0x0400 },    /* The bootloader is out of the normal flash. */
    { "at89c5131",      tar_at89c5131,      adc_8051,  0x2FFD, 0x03eb, 0x08000, 0x0000, true,  128, false, true,  128, 0x0400 },    /* The bootloader is out of the normal flash. */
    { "at89c5132",      tar_at89c5132,      adc_8051,  0x2FFF, 0x03eb, 0x10000, 0x0C00, true,  128, false, true,  0,   0      },
    { "at90usb1287",    tar_at90usb1287,    adc_AVR,   0x2FFB, 0x03eb, 0x20000, 0x2000, true,  128, true,  false, 128, 0x1000 },
    { "at90usb1286",    tar_at90usb1286,    adc_AVR,   0x2FFB, 0x03eb, 0x20000, 0x2000, true,  128, true,  false, 128, 0x1000 },
    { "at90usb1287-4k", tar_at90usb1287_4k, adc_AVR,   0x2FFB, 0x03eb, 0x20000, 0x1000, true,  128, true,  false, 128, 0x1000 },
    { "at90usb1286-4k", tar_at90usb1286_4k, adc_AVR,   0x2FFB, 0x03eb, 0x20000, 0x1000, true,  128, true,  false, 128, 0x1000 },
    { "at90usb647",     tar_at90usb647,     adc_AVR,   0x2FF9, 0x03eb, 0x10000, 0x2000, true,  128, true,  false, 128, 0x0800 },
    { "at90usb646",     tar_at90usb646,     adc_AVR,   0x2FF9, 0x03eb, 0x10000, 0x2000, true,  128, true,  false, 128, 0x0800 },
    { "at90usb162",     tar_at90usb162,     adc_AVR,   0x2FFA, 0x03eb, 0x04000, 0x1000, true,  128, true,  false, 128, 0x0200 },
    { "at90usb82",      tar_at90usb82,      adc_AVR,   0x2FF7, 0x03eb, 0x02000, 0x1000, true,  128, true,  false, 128, 0x0200 },
    { "atmega32u6",     tar_atmega32u6,     adc_AVR,   0x2FF2, 0x03eb, 0x08000, 0x1000, true,  128, true,  false, 128, 0x0400 },
    { "atmega32u4",     tar_atmega32u4,     adc_AVR,   0x2FF4, 0x03eb, 0x08000, 0x1000, true,  128, true,  false, 128, 0x0400 },
    { "atmega32u2",     tar_atmega32u2,     adc_AVR,   0x2FF0, 0x03eb, 0x08000, 0x1000, true,  128, true, false,  128, 0x0400 },
    { "atmega16u2",     tar_atmega16u2,     adc_AVR,   0x2FEF, 0x03eb, 0x04000, 0x1000, true,  128, true,  false, 128, 0x0200 },
    { "atmega16u4",     tar_atmega16u4,     adc_AVR,   0x2FF3, 0x03eb, 0x04000, 0x1000, true,  128, true,  false, 128, 0x0200 },
    { "atmega8u2",      tar_atmega8u2,      adc_AVR,   0x2FEE, 0x03eb, 0x02000, 0x1000, true,  128, true,  false, 128, 0x0200 },
    { "at32uc3a0128",   tar_at32uc3a0128,   adc_AVR32, 0x2FF8, 0x03eb, 0x20000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3a1128",   tar_at32uc3a1128,   adc_AVR32, 0x2FF8, 0x03eb, 0x20000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3a0256",   tar_at32uc3a0256,   adc_AVR32, 0x2FF8, 0x03eb, 0x40000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3a1256",   tar_at32uc3a1256,   adc_AVR32, 0x2FF8, 0x03eb, 0x40000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3a0512",   tar_at32uc3a0512,   adc_AVR32, 0x2FF8, 0x03eb, 0x80000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3a1512",   tar_at32uc3a1512,   adc_AVR32, 0x2FF8, 0x03eb, 0x80000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3a0512es", tar_at32uc3a0512es, adc_AVR32, 0x2FF8, 0x03eb, 0x80000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3a1512es", tar_at32uc3a1512es, adc_AVR32, 0x2FF8, 0x03eb, 0x80000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3a364",    tar_at32uc3a364,    adc_AVR32, 0x2FF1, 0x03eb, 0x10000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3a364s",   tar_at32uc3a364s,   adc_AVR32, 0x2FF1, 0x03eb, 0x10000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3a3128",   tar_at32uc3a3128,   adc_AVR32, 0x2FF1, 0x03eb, 0x20000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3a3128s",  tar_at32uc3a3128s,  adc_AVR32, 0x2FF1, 0x03eb, 0x20000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3a3256",   tar_at32uc3a3256,   adc_AVR32, 0x2FF1, 0x03eb, 0x40000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3a3256s",  tar_at32uc3a3256s,  adc_AVR32, 0x2FF1, 0x03eb, 0x40000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3b064",    tar_at32uc3b064,    adc_AVR32, 0x2FF6, 0x03eb, 0x10000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3b164",    tar_at32uc3b164,    adc_AVR32, 0x2FF6, 0x03eb, 0x10000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3b0128",   tar_at32uc3b0128,   adc_AVR32, 0x2FF6, 0x03eb, 0x20000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3b1128",   tar_at32uc3b1128,   adc_AVR32, 0x2FF6, 0x03eb, 0x20000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3b0256",   tar_at32uc3b0256,   adc_AVR32, 0x2FF6, 0x03eb, 0x40000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3b1256",   tar_at32uc3b1256,   adc_AVR32, 0x2FF6, 0x03eb, 0x40000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3b0256es", tar_at32uc3b0256es, adc_AVR32, 0x2FF6, 0x03eb, 0x40000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3b1256es", tar_at32uc3b1256es, adc_AVR32, 0x2FF6, 0x03eb, 0x40000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3b0512",   tar_at32uc3b0512,   adc_AVR32, 0x2FF6, 0x03eb, 0x80000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3b1512",   tar_at32uc3b1512,   adc_AVR32, 0x2FF6, 0x03eb, 0x80000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3c064",    tar_at32uc3c064,    adc_AVR32, 0x2FEB, 0x03eb, 0x10000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3c0128",   tar_at32uc3c0128,   adc_AVR32, 0x2FEB, 0x03eb, 0x20000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3c0256",   tar_at32uc3c0256,   adc_AVR32, 0x2FEB, 0x03eb, 0x40000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3c0512",   tar_at32uc3c0512,   adc_AVR32, 0x2FEB, 0x03eb, 0x80000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3c164",    tar_at32uc3c164,    adc_AVR32, 0x2FEB, 0x03eb, 0x10000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3c1128",   tar_at32uc3c1128,   adc_AVR32, 0x2FEB, 0x03eb, 0x20000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3c1256",   tar_at32uc3c1256,   adc_AVR32, 0x2FEB, 0x03eb, 0x40000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3c1512",   tar_at32uc3c1512,   adc_AVR32, 0x2FEB, 0x03eb, 0x80000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3c264",    tar_at32uc3c264,    adc_AVR32, 0x2FEB, 0x03eb, 0x10000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3c2128",   tar_at32uc3c2128,   adc_AVR32, 0x2FEB, 0x03eb, 0x20000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3c2256",   tar_at32uc3c2256,   adc_AVR32, 0x2FEB, 0x03eb, 0x40000, 0x2000, false, 512, false, true,  0,   0      },
    { "at32uc3c2512",   tar_at32uc3c2512,   adc_AVR32, 0x2FEB, 0x03eb, 0x80000, 0x2000, false, 512, false, true,  0,   0      },
    { NULL }
};

/* ----- command specific structures ---------------------------------------- */
static struct option_mapping_structure command_map[] = {
    { "configure",    com_configure },
    { "dump",         com_dump      },
    { "dump-eeprom",  com_edump     },
    { "dump-user",    com_udump     },
    { "erase",        com_erase     },
    { "flash",        com_flash     },
    { "flash-user",   com_user      },
    { "flash-eeprom", com_eflash    },
    { "get",          com_get       },
    { "getfuse",      com_getfuse   },
    { "setfuse",      com_setfuse   },
    { "reset",        com_reset     },
    { "start",        com_start_app },
    { "version",      com_version   },
    { NULL }
};

/* ----- configure specific structures -------------------------------------- */
static struct option_mapping_structure configure_map[] = {
    { "BSB", conf_BSB },
    { "SBV", conf_SBV },
    { "SSB", conf_SSB },
    { "EB",  conf_EB  },
    { "HSB", conf_HSB },
    { NULL }
};

/* ----- get specific structures -------------------------------------- */
static struct option_mapping_structure get_map[] = {
    { "bootloader-version", get_bootloader   },
    { "ID1",                get_ID1          },
    { "ID2",                get_ID2          },
    { "BSB",                get_BSB          },
    { "SBV",                get_SBV          },
    { "SSB",                get_SSB          },
    { "EB",                 get_EB           },
    { "manufacturer",       get_manufacturer },
    { "family",             get_family       },
    { "product-name",       get_product_name },
    { "product-revision",   get_product_rev  },
    { "HSB",                get_HSB          },
    { NULL }
};

/* ----- getfuse specific structures ---------------------------------- */
static struct option_mapping_structure getfuse_map[] = {
    { "LOCK",           get_lock           },
    { "EPFL",           get_epfl           },
    { "BOOTPROT",       get_bootprot       },
    { "BODLEVEL",       get_bodlevel       },
    { "BODHYST",        get_bodhyst        },
    { "BODEN",          get_boden          },
    { "ISP_BOD_EN",     get_isp_bod_en     },
    { "ISP_IO_COND_EN", get_isp_io_cond_en },
    { "ISP_FORCE",      get_isp_force      },
    { NULL }
};

/* ----- setfuse specific structures ---------------------------------- */
static struct option_mapping_structure setfuse_map[] = {
    { "LOCK",           set_lock           },
    { "EPFL",           set_epfl           },
    { "BOOTPROT",       set_bootprot       },
    { "BODLEVEL",       set_bodlevel       },
    { "BODHYST",        set_bodhyst        },
    { "BODEN",          set_boden          },
    { "ISP_BOD_EN",     set_isp_bod_en     },
    { "ISP_IO_COND_EN", set_isp_io_cond_en },
    { "ISP_FORCE",      set_isp_force      },
    { NULL }
};


static void usage()
{
    struct target_mapping_structure *map = NULL;

    map = target_map;

    fprintf( stderr, PACKAGE_STRING "\n");
    fprintf( stderr, "Usage: dfu-programmer target command [command-options] "
                     "[global-options] [file|data]\n" );
    fprintf( stderr, "targets:\n" );
    while( 0 != *((int32_t *) map) ) {
        fprintf( stderr, "        %s\n", map->name );
        map++;
    }
    fprintf( stderr, "global-options: --quiet, --debug level\n" );
    fprintf( stderr, "commands:\n" );
    fprintf( stderr, "        configure {BSB|SBV|SSB|EB|HSB} "
                     "[--suppress-validation] [global-options] data\n" );
    fprintf( stderr, "        dump "
                     "[global-options]\n" );
    fprintf( stderr, "        dump-eeprom "
                     "[global-options]\n" );
    fprintf( stderr, "        dump-user "
                     "[global-options]\n" );
    fprintf( stderr, "        erase "
                     "[--suppress-validation] [global-options]\n" );
    fprintf( stderr, "        flash "
                     "[--suppress-validation] [--suppress-bootloader-mem] [global-options] {file|STDIN}\n" );
    fprintf( stderr, "        flash-eeprom "
                     "[--suppress-validation] [global-options] {file|STDIN}\n" );
    fprintf( stderr, "        flash-user "
                     "[--suppress-validation] [global-options] {file|STDIN}\n" );
    fprintf( stderr, "        get {bootloader-version|ID1|ID2|BSB|SBV|SSB|EB|\n"
                     "            manufacturer|family|product-name|\n"
                     "            product-revision|HSB} "
                     "[global-options]\n" );
    fprintf( stderr, "        getfuse {LOCK|EPFL|BOOTPROT|BODLEVEL|BODHYST|\n"
                     "                BODEN|ISP_BOD_EN|ISP_IO_COND_EN|\n"
                     "                ISP_FORCE} "
                     "[global-options]\n" );
    fprintf( stderr, "        setfuse {LOCK|EPFL|BOOTPROT|BODLEVEL|BODHYST|\n"
                     "                BODEN|ISP_BOD_EN|ISP_IO_COND_EN|\n"
                     "                ISP_FORCE} "
                     "[global-options] data\n" );
    fprintf( stderr, "        reset [global-options]\n" );
    fprintf( stderr, "        start [global-options]\n" );
    fprintf( stderr, "        version [global-options]\n" );
}

static int32_t assign_option( int32_t *arg,
                              char *value,
                              struct option_mapping_structure *map )
{
    while( 0 != *((int32_t *) map) ) {
        if( 0 == strcasecmp(value, map->name) ) {
            *arg = map->value;
            return 0;
        }

        map++;
    }

    return -1;
}


static int32_t assign_target( struct programmer_arguments *args,
                              char *value,
                              struct target_mapping_structure *map )
{
    while( 0 != *((int32_t *) map) ) {
        if( 0 == strcasecmp(value, map->name) ) {
            args->target  = map->value;
            args->chip_id = map->chip_id;
            args->vendor_id = map->vendor_id;
            args->device_type = map->device_type;
            args->eeprom_memory_size = map->eeprom_memory_size;
            args->flash_page_size = map->flash_page_size;
            args->eeprom_page_size = map->eeprom_page_size;
            args->initial_abort = map->initial_abort;
            args->honor_interfaceclass = map->honor_interfaceclass;
            args->memory_address_top = map->memory_size - 1;
            args->memory_address_bottom = 0;
            args->flash_address_top = args->memory_address_top;
            args->flash_address_bottom = args->memory_address_bottom;
            args->bootloader_bottom = 0;
            args->bootloader_top = 0;
            args->bootloader_at_highmem = map->bootloader_at_highmem;
            if( true == map->bootloader_at_highmem ) {
                args->bootloader_bottom = map->memory_size - map->bootloader_size;
                args->bootloader_top = args->flash_address_top;
                args->flash_address_top -= map->bootloader_size;
            } else {
                args->bootloader_bottom = args->flash_address_bottom;
                args->bootloader_top += map->bootloader_size - 1;
                args->flash_address_bottom += map->bootloader_size;
            }
            switch( args->device_type ) {
                case adc_8051:
                    strncpy( args->device_type_string, "8051",
                             DEVICE_TYPE_STRING_MAX_LENGTH );
                    break;
                case adc_AVR:
                    strncpy( args->device_type_string, "AVR",
                             DEVICE_TYPE_STRING_MAX_LENGTH );
                    break;
                case adc_AVR32:
                    strncpy( args->device_type_string, "AVR32",
                             DEVICE_TYPE_STRING_MAX_LENGTH );
                    break;
            }
            return 0;
        }

        map++;
    }

    return -1;
}


static int32_t assign_global_options( struct programmer_arguments *args,
                                      const size_t argc,
                                      char **argv )
{
    size_t i = 0;

    /* Find '--quiet' if it is here */
    for( i = 0; i < argc; i++ ) {
        if( 0 == strcmp("--quiet", argv[i]) ) {
            *argv[i] = '\0';
            args->quiet = 1;
            break;
        }
    }

    /* Find '--suppress-bootloader-mem' if it is here */
    for( i = 0; i < argc; i++ ) {
        if( 0 == strcmp("--suppress-bootloader-mem", argv[i]) ) {
            *argv[i] = '\0';
            args->suppressbootloader = 1;
            break;
        }
    }

    /* Find '--suppress-validation' if it is here - even though it is not
     * used by all this is easier. */
    for( i = 0; i < argc; i++ ) {
        if( 0 == strcmp("--suppress-validation", argv[i]) ) {
            *argv[i] = '\0';

            switch( args->command ) {
                case com_configure:
                    args->com_configure_data.suppress_validation = 1;
                    break;
                case com_erase:
                    args->com_erase_data.suppress_validation = 1;
                    break;
                case com_flash:
                case com_eflash:
                case com_user:
                    args->com_flash_data.suppress_validation = 1;
                    break;
                default:
                    /* not supported. */
                    return -1;
            }

            break;
        }
    }


    /* Find '--debug' if it is here */
    for( i = 0; i < argc; i++ ) {
        if( 0 == strncmp("--debug", argv[i], 7) ) {

            if( 0 == strncmp("--debug=", argv[i], 8) ) {
                if( 1 != sscanf(argv[i], "--debug=%i", &debug) )
                    return -2;
            } else {
                if( (i+1) >= argc )
                    return -3;

                if( 1 != sscanf(argv[i+1], "%i", &debug) )
                    return -4;

                *argv[i+1] = '\0';
            }
            *argv[i] = '\0';
            break;
        }
    }

    return 0;
}

static int32_t assign_com_setfuse_option( struct programmer_arguments *args,
                                            const int32_t parameter,
                                            char *value )
{
    /* name & value */
    if( 0 == parameter ) {
        /* name */
        if( 0 != assign_option((int32_t *) &(args->com_setfuse_data.name),
                               value, setfuse_map) )
        {
            return -1;
        }
    } else {
        int32_t temp = 0;
        /* value */
        if( 1 != sscanf(value, "%i", &(temp)) )
            return -2;

        /* ensure the range is greater than 0 */
        if( temp < 0 )
            return -3;

        args->com_setfuse_data.value = temp;
    }

    return 0;
}



static int32_t assign_com_configure_option( struct programmer_arguments *args,
                                            const int32_t parameter,
                                            char *value )
{
    /* name & value */
    if( 0 == parameter ) {
        /* name */
        if( 0 != assign_option((int32_t *) &(args->com_configure_data.name),
                               value, configure_map) )
        {
            return -1;
        }
    } else {
        int32_t temp = 0;
        /* value */
        if( 1 != sscanf(value, "%i", &(temp)) )
            return -2;

        /* ensure the range is greater than 0 */
        if( temp < 0 )
            return -3;

        args->com_configure_data.value = temp;
    }

    return 0;
}


static int32_t assign_com_flash_option( struct programmer_arguments *args,
                                        const int32_t parameter,
                                        char *value )
{
    /* file */
    args->com_flash_data.original_first_char = *value;
    args->com_flash_data.file = value;

    return 0;
}

static int32_t assign_com_getfuse_option( struct programmer_arguments *args,
                                      const int32_t parameter,
                                      char *value )
{
    /* name */
    if( 0 != assign_option((int32_t *) &(args->com_getfuse_data.name),
                           value, getfuse_map) )
    {
        return -1;
    }

    return 0;
}



static int32_t assign_com_get_option( struct programmer_arguments *args,
                                      const int32_t parameter,
                                      char *value )
{
    /* name */
    if( 0 != assign_option((int32_t *) &(args->com_get_data.name),
                           value, get_map) )
    {
        return -1;
    }

    return 0;
}


static int32_t assign_command_options( struct programmer_arguments *args,
                                       const size_t argc,
                                       char **argv )
{
    size_t i = 0;
    int32_t param = 0;
    int32_t required_params = 0;

    /* Deal with all remaining command-specific arguments. */
    for( i = 0; i < argc; i++ ) {
        if( '\0' == *argv[i] )
            continue;

        switch( args->command ) {
            case com_configure:
                required_params = 2;
                if( 0 != assign_com_configure_option(args, param, argv[i]) )
                    return -1;
                break;

            case com_setfuse:
                required_params = 2;
                if( 0 != assign_com_setfuse_option(args, param, argv[i]) )
                    return -1;
                break;

            case com_flash:
            case com_eflash:
            case com_user:
                required_params = 1;
                if( 0 != assign_com_flash_option(args, param, argv[i]) )
                    return -3;
                break;

            case com_getfuse:
                required_params = 1;
                if( 0 != assign_com_getfuse_option(args, param, argv[i]) )
                    return -4;
                break;
            case com_get:
                required_params = 1;
                if( 0 != assign_com_get_option(args, param, argv[i]) )
                    return -4;
                break;

            default:
                return -5;
        }

        *argv[i] = '\0';
        param++;
    }

    if( required_params != param )
        return -6;

    return 0;
}


static void print_args( struct programmer_arguments *args )
{
    const char *command = "(unknown)";
    const char *target = "(unknown)";
    size_t i;

    for( i = 0; i < sizeof(target_map) / sizeof(target_map[0]); i++ ) {
        if( args->target == target_map[i].value ) {
            target = target_map[i].name;
            break;
        }
    }

    for( i = 0; i < sizeof(command_map) / sizeof(command_map[0]); i++ ) {
        if( args->command == command_map[i].value ) {
            command = command_map[i].name;
            break;
        }
    }

    fprintf( stderr, "     target: %s\n", target );
    fprintf( stderr, "    chip_id: 0x%04x\n", args->chip_id );
    fprintf( stderr, "  vendor_id: 0x%04x\n", args->vendor_id );
    fprintf( stderr, "    command: %s\n", command );
    fprintf( stderr, "      quiet: %s\n", (0 == args->quiet) ? "false" : "true" );
    fprintf( stderr, "      debug: %d\n", debug );
    fprintf( stderr, "device_type: %s\n", args->device_type_string );
    fprintf( stderr, "------ command specific below ------\n" );

    switch( args->command ) {
        case com_configure:
            fprintf( stderr, "       name: %d\n", args->com_configure_data.name );
            fprintf( stderr, "   validate: %s\n",
                     (args->com_configure_data.suppress_validation) ?
                        "false" : "true" );
            fprintf( stderr, "      value: %d\n", args->com_configure_data.value );
            break;
        case com_erase:
            fprintf( stderr, "   validate: %s\n",
                     (args->com_erase_data.suppress_validation) ?
                        "false" : "true" );
            break;
        case com_flash:
        case com_eflash:
            fprintf( stderr, "   validate: %s\n",
                     (args->com_flash_data.suppress_validation) ?
                        "false" : "true" );
            fprintf( stderr, "   hex file: %s\n", args->com_flash_data.file );
            break;
        case com_get:
            fprintf( stderr, "       name: %d\n", args->com_get_data.name );
            break;
        default:
            break;
    }
    fprintf( stderr, "\n" );
    fflush( stdout );
}


int32_t parse_arguments( struct programmer_arguments *args,
                         const size_t argc,
                         char **argv )
{
    int32_t i;
    int32_t status = 0;

    if( NULL == args )
        return -1;

    /* initialize the argument block to empty, known values */
    args->target  = tar_none;
    args->command = com_none;
    args->quiet   = 0;
    args->suppressbootloader = 0;

    /* Make sure there are the minimum arguments */
    if( argc < 3 ) {
        status = -2;
        goto done;
    }

    if( 0 != assign_target(args, argv[1], target_map) ) {
        status = -3;
        goto done;
    }

    if( 0 != assign_option((int32_t *) &(args->command), argv[2], command_map) ) {
        status = -4;
        goto done;
    }

    /* These were taken care of above. */
    *argv[0] = '\0';
    *argv[1] = '\0';
    *argv[2] = '\0';

    if( 0 != assign_global_options(args, argc, argv) ) {
        status = -5;
        goto done;
    }

    if( 0 != assign_command_options(args, argc, argv) ) {
        status = -6;
        goto done;
    }

    /* Make sure there weren't any *extra* options. */
    for( i = 0; i < argc; i++ ) {
        if( '\0' != *argv[i] ) {
            fprintf( stderr, "unrecognized parameter\n" );
            status = -7;
            goto done;
        }
    }

    /* if this is a flash command, restore the filename */
    if( (com_flash == args->command) || (com_eflash == args->command) || (com_user == args->command) ) {
        if( 0 == args->com_flash_data.file ) {
            fprintf( stderr, "flash filename is missing\n" );
            status = -8;
            goto done;
        }
        args->com_flash_data.file[0] = args->com_flash_data.original_first_char;
    }

done:
    if( 1 < debug ) {
        print_args( args );
    }

    if( 0 != status ) {
        usage();
    }

    return status;
}
