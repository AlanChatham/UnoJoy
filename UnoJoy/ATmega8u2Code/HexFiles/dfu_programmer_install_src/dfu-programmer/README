dfu-programmer is an implementation of the Device Firmware Upgrade class
USB driver that enables firmware upgrades for various USB enabled (with the
correct bootloader) Atmel chips.  This program was created because the
Atmel "FLIP" program for flashing devices does not support flashing via USB
on Linux, and because standard DFU loaders do not work for Atmel's chips.

Check out the Atmel website for more information.  They are kind enough to
provide generally correct specifications this implementation is based on.

The project website is http://dfu-programmer.sourceforge.net and you can
use that to check for updates.

Currently Supported Chips
=========================
8051 based:
  at89c51snd1c
  at89c5130
  at89c5131
  at89c5132

AVR based:
  at90usb1287
  at90usb1286
  at90usb647
  at90usb646

Simple install procedure
========================

  % tar -xzf dfu-programmer-<version>.tar.gz    # unpack the sources
  % cd dfu-programmer                           # change to the top-level
                                                # directory

  [ If the source was checked-out from  CVS, run the following command ]
  % ./bootstrap.sh                              # regenerate base config
                                                # files
  
  % ./configure                                 # regenerate configure and
                                                # run it

  [ Optionally you can specify where dfu-programmer gets installed
    using the --prefix= option to the ./configure command.  See
    % ./configure --help for more details. ]

  % make                                        # build dfu-programmer
  [ Become root if necessary ]
  % make install                                # install dfu-programmer

Building RPM Binary Packages
============================
This section is intended to make it easier for those people that wish to
build RPMs from the source included in this package, but aren't sure how.

1) Copy dfu-programmer-<version>.tar.gz to your RPM SOURCES directory. Usually
   this is /usr/src/redhat/SOURCES/.

2) Extract or copy dfu-programmer.spec into your RPM SPECS directory. Usually
   this is /usr/src/redhat/SPECS/.

3) In your RPM SPECS directory, issue the command 'rpm -ba dfu-programmer.spec'.
   This will cause rpm to extract the dfu-programmer sources to a temporary
   directory, build them, and build rpm packages based on the information in
   the spec file. The binary rpms will be put into your RPM RPMS directory.
   Usually this is /usr/src/redhat/RPMS/<platform>/.

If you have any further questions, please refer to the RPM documentation.
