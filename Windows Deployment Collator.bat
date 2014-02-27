@echo off
if exist UnoJoy.h (
  echo Found UnoJoy.
  echo.
  echo Collecting Windows files..
  REM Don't add in the OSX dfu-programmer files
  xcopy /i /y "ATmega8u2Code\HexFiles\*.hex" "UnoJoyWin\ATmega8u2Code\HexFiles"
  xcopy /i /y "ATmega8u2Code\HexFiles\*.exe" "UnoJoyWin\ATmega8u2Code\HexFiles"
  
  xcopy /i /y "ATmega8u2Code" "UnoJoyWin/ATmega8u2Code"
  xcopy /i /y /s Drivers "UnoJoyWin/Drivers"
  xcopy /i /y /s Examples "UnoJoyWin/Examples"
  xcopy /i /y /s UnoJoyArduinoSample "UnoJoyWin/UnoJoyArduinoSample"
  
  REM Don't add in the OSX visualizer files
  xcopy /i /y "UnoJoyProcessingVisualizer/data" "UnoJoyWin/UnoJoyProcessingVisualizer/data"
  xcopy /i /y "UnoJoyProcessingVisualizer/lib" "UnoJoyWin/UnoJoyProcessingVisualizer/lib"
  xcopy /i /y "UnoJoyProcessingVisualizer" "UnoJoyWin/UnoJoyProcessingVisualizer"
  
  REM Only take the Windows root files
  
  copy /y README-GettingStarted.txt "UnoJoyWin/README-GettingStarted.txt" 
  copy /y TurnIntoAJoystick.bat "UnoJoyWin/TurnIntoAJoystick.bat" 
  copy /y TurnIntoAnArduino.bat "UnoJoyWin/TurnIntoAnArduino.bat" 
  copy /y UnoJoy.h "UnoJoyWin/UnoJoy.h" 
  copy /y UnoJoyDriverInstaller.bat "UnoJoyWin/UnoJoyDriverInstaller.bat" 
  
) else (
  echo Didn't find UnoJoy...
  echo Make sure you are running this file from the root of your UnoJoy directory
)
echo Press any key to exit...
pause > nul