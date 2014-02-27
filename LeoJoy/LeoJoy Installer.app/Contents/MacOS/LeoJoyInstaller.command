#!/bin/sh
echo
echo
echo "Installing LeoJoy!"
echo 

#Get to the current directory of the .app
APP_DIR="$(dirname "$0")"
cd "$APP_DIR"

# Get back to the directory that holds the .app bundle
cd ../../..

#Make sure the Arduino app exists...
if [ -e "Arduino.app" ] ; then
    echo "Found Arduino, installing LeoJoy!"
    echo
else
    echo "Couldn't find Arduino.app... Did you run this installer in the same folder as Arduino.app?"
    pwd
    exit
fi

ARDUINO_DIR="Arduino.app/Contents/Resources/Java/"

# Start copying files
cp -R "$APP_DIR/hardware" "$ARDUINO_DIR"
cp -R "$APP_DIR/examples" "$ARDUINO_DIR"

# Append the LeoJoy! files to the boards.txt file
cd "$ARDUINO_DIR/hardware/arduino"

#Test if LeoJoy has already been added to boards.txt
grep LeoJoy boards.txt
if [ $? -eq 0 ] ; then
    echo "Looks like LeoJoy! is already installed in your list of boards, so we just left that there."
else
    cat boards.txt leoboard.txt > temp.txt
    yes | cp temp.txt boards.txt
    rm temp.txt
    rm leoboard.txt
fi

echo
echo
echo "LeoJoy! setup is complete! If it was open, restart Arduino, and you'll be able to pick LeoJoy! under the Boards… menu. Thanks!"
echo
echo