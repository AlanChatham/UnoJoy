@echo off
if exist UnoJoy.h (
  echo Found UnoJoy.
  echo.
  echo Collecting OSX files..
  REM Don't add in the Windows batchisp files
  xcopy /i /y "ATmega8u2Code\HexFiles\*.hex" "UnoJoyOSX\ATmega8u2Code\HexFiles"
  xcopy /i /y "ATmega8u2Code\HexFiles\dfu-*" "UnoJoyOSX\ATmega8u2Code\HexFiles"
  
  xcopy /i /y "ATmega8u2Code" "UnoJoyOSX/ATmega8u2Code"
  xcopy /i /y /s "LionUnoJoyDrivers.pkg" "UnoJoyOSX/LionUnoJoyDrivers.pkg"
  xcopy /i /y /s "SnowLeopardUnoJoyDrivers.pkg" "UnoJoyOSX/SnowLeopardUnoJoyDrivers.pkg"
  xcopy /i /y /s Examples "UnoJoyOSX/Examples"
  xcopy /i /y /s UnoJoyArduinoSample "UnoJoyOSX/UnoJoyArduinoSample"
  
  REM Don't add in the Windows visualizer files
  xcopy /i /y "UnoJoyProcessingVisualizer/data" "UnoJoyOSX/UnoJoyProcessingVisualizer/data"
  xcopy /i /y /s "UnoJoyProcessingVisualizer/UnoJoyProcessingVisualizer.app" "UnoJoyOSX/UnoJoyProcessingVisualizer/UnoJoyProcessingVisualizer.app"
  xcopy /i /y "UnoJoyProcessingVisualizer\*.pde" "UnoJoyOSX/UnoJoyProcessingVisualizer"
  
  REM Only take the OSX root files
  
  copy /y README-GettingStarted.txt "UnoJoyOSX/README-GettingStarted.txt" 
  copy /y TurnIntoAJoystick.bat "UnoJoyOSX/TurnIntoAJoystick.command" 
  copy /y TurnIntoAnArduino.bat "UnoJoyOSX/TurnIntoAnArduino.command" 
  copy /y UnoJoy.h "UnoJoyOSX/UnoJoy.h"
  
) else (
  echo Didn't find UnoJoy...
  echo Make sure you are running this file from the root of your UnoJoy directory
)
echo Press any key to exit...
pause > nul