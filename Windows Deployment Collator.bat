@echo off
set archiveName=UnoJoyWin-%date:~-10,2%-%date:~-7,2%-%date:~-4,4%.zip
if exist "UnoJoy/UnoJoy.h" (
  echo Found UnoJoy.
  echo.
  echo Collecting Windows files..
  REM Don't add in the OSX dfu-programmer files
  xcopy /i /y "UnoJoy\ATmega8u2Code\HexFiles\*.hex" "UnoJoyWin\ATmega8u2Code\HexFiles"
  xcopy /i /y "UnoJoy\ATmega8u2Code\HexFiles\*.exe" "UnoJoyWin\ATmega8u2Code\HexFiles"
  
  xcopy /i /y "UnoJoy/ATmega8u2Code" "UnoJoyWin/ATmega8u2Code"
  xcopy /i /y /s "Drivers/Windows" "UnoJoyWin/Drivers/Windows"
  xcopy /i /y /s "UnoJoy/Examples" "UnoJoyWin/Examples"
  xcopy /i /y /s "UnoJoy/UnoJoyArduinoSample" "UnoJoyWin/UnoJoyArduinoSample"
  
  REM Don't add in the OSX visualizer files
  xcopy /i /y "UnoJoy/UnoJoyProcessingVisualizer/data" "UnoJoyWin/UnoJoyProcessingVisualizer/data"
  xcopy /i /y "UnoJoy/UnoJoyProcessingVisualizer/lib" "UnoJoyWin/UnoJoyProcessingVisualizer/lib"
  xcopy /i /y "UnoJoy/UnoJoyProcessingVisualizer" "UnoJoyWin/UnoJoyProcessingVisualizer"
  
  REM Only take the Windows root files
  copy /y "README.md" "UnoJoyWin/README.txt" 
  copy /y "UnoJoy\TurnIntoAJoystick.bat" "UnoJoyWin/TurnIntoAJoystick.bat" 
  copy /y "UnoJoy\TurnIntoAnArduino.bat" "UnoJoyWin/TurnIntoAnArduino.bat" 
  copy /y "UnoJoy\UnoJoy.h" "UnoJoyWin/UnoJoy.h" 
  copy /y "Drivers\UnoJoyDriverInstaller.bat" "UnoJoyWin/UnoJoyDriverInstaller.bat" 
  
  7z a %archiveName% "UnoJoyWin"
  
) else (
  echo Didn't find UnoJoy...
  echo Make sure you are running this file from the root of your UnoJoy directory
)
echo Press any key to exit...
echo %archiveName%
pause > nul