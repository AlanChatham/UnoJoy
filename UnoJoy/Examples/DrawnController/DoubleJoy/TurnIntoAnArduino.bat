@echo off
if not exist ATmega8u2Code/HexFiles/batchisp.exe (
echo.
echo The program used to flash the memory couldn't be found...
echo Did you move this .bat file?  Or delete something in the ATmega8u2Code folder?
echo Press any key to exit...
goto EXIT
)
echo.
echo Like magic.
cd ATmega8u2Code/HexFiles
echo Attempting to re-flash for an Arduino Uno R1/R2
@echo on
batchisp -device at90usb82 -hardware usb -operation erase f memory flash blankcheck loadbuffer "Arduino-usbserial-uno.hex" program verify start reset 0
@echo off
if %errorlevel% NEQ 0 (
    goto R3FLASH
)
else (
    goto SUCCESS
)

:R3FLASH
echo Trying to re-flash for an Arduino Uno R3
@echo on
batchisp -device atmega16u2 -hardware usb -operation erase f memory flash blankcheck loadbuffer "Arduino-usbserial-uno.hex" program verify start reset 0
@echo off
if %errorlevel% NEQ 0 (
    echo %errorlevel%
    echo The firmware was NOT loaded...
    echo.
    echo Did you install the Atmel FLIP program? - http://www.atmel.com/tools/FLIP.aspx -
    echo Is the Arduino plugged in?
    echo Is is it in 'Arduino UNO DFU' mode?
    echo Press any key to exit..
    goto EXIT
) else (
    goto SUCCESS
)

:SUCCESS
echo Now, you need to unplug the Arduino and plug it back in,
echo and it'll show back up as an Arduino. Press any key to exit....
goto EXIT


:EXIT
pause > nul