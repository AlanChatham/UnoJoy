#!/bin/sh
cd "`dirname "$0"`"
echo # New Line
echo 'Like magic. '
echo # New Line
cd ATmega8u2Code/HexFiles
if test -e ./dfu-programmer; then
   echo "Trying to program for Arduino Uno R1/R2..."
  ./dfu-programmer at90usb82 flash Arduino-usbserial-uno.hex
  if test $? != 0; then
    echo "We didn't find the R1/R2 model, checking for an R3..."
    ./dfu-programmer atmega16u2 flash Arduino-usbserial-uno.hex
    if test $? != 0; then
      echo # New line
      echo "dfu-programmer couldn't connect to the Arduino..."
      echo "Did you put the Arduino into DFU mode?"
      echo "Did you install the libusb driver?"
    else
       echo # New Line
       echo "Unplug your Arduino and plug it back in. It's back to being an Arduino now!"
    fi
  else
    echo # New Line
    echo "Unplug your Arduino and plug it back in. It's back to being an Arduino now!"
  fi
else
  echo "Couldn't find dfu-programmer..."
  echo "Did you move this script or do anything to the ATmeg8u2Code directory?"
fi


