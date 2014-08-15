#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
#AC_INIT(dfu-programmer, VERSION, BUG-REPORT-ADDRESS)
AC_INIT([dfu-programmer],[0.5.2])
AC_CONFIG_AUX_DIR(m4)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AC_CONFIG_SRCDIR([src/atmel.c])
AM_CONFIG_HEADER([config.h])

AM_MAINTAINER_MODE

# Checks for programs.
AC_PROG_CC

# Checks for libusb - from sane-backends configuration

dnl Enable libusb-1.0, if available
AC_ARG_ENABLE(libusb_1_0,
	AC_HELP_STRING([--disable-libusb_1_0], [disable libusb-1.0 support if available]),
	[disable_libusb_1_0=$enableval], [disable_libusb_1_0=no])

dnl If libusb-1.0 is enabled and available, prefer that to the old libusb
have_libusb_1_0=no
if test "$disable_libusb_1_0" = "no"; then
  PKG_CHECK_MODULES(LIBUSB_1_0, [ libusb-1.0 >= 1.0.0 ], have_libusb_1_0=yes, have_libusb_1_0=no)
  if test "$have_libusb_1_0" = "yes"; then
    CFLAGS="$CFLAGS $LIBUSB_1_0_CFLAGS"
    LDFLAGS="$LDFLAGS $LIBUSB_1_0_LIBS"
    HAVE_USB=yes
  fi
fi

if test "$have_libusb_1_0" = "no"; then
  dnl Fallback to the old libusb
  dnl libusb >= 0.1.8 is required, as we need usb_interrupt_read()
  AC_CHECK_HEADER(usb.h,
                  AC_CHECK_LIB(usb, usb_interrupt_read, 
                               [LDFLAGS="$LDFLAGS -lusb"
                                HAVE_USB=yes]))
fi

dnl The following logic is useful for distributions.  If they force
dnl USB support with --enable-libusb=yes then configure will fail
dnl when its detected that it can not be supported.  Default is
dnl "auto" which means only enable if libraries are found and do
dnl not error out.
if test "$HAVE_USB" = "no"; then
  AC_MSG_ERROR([USB support requested but required libraries not found.])
fi
if test "$HAVE_USB" = "yes"; then
  if test "$have_libusb_1_0" = "yes"; then
    AC_DEFINE(HAVE_LIBUSB_1_0, 1, [Define to 1 if you have libusb-1.0.])
  else
    AC_DEFINE(HAVE_LIBUSB, 1, [Define to 1 if you have libusb.])
  fi
fi


# Checks for header files.
AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_CHECK_FUNCS([memset])

AC_CONFIG_FILES(fedora/dfu-programmer.spec Makefile docs/Makefile src/Makefile)
AC_OUTPUT
