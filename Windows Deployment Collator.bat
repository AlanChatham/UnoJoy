REM This is a batch file to automate the process of making
REM  an easy-to-download copy of the Windows UnoJoy! files.
REM  It only works on Windows, and requires that you have
REM  7-Zip installed on your system. If you don't have 7-Zip,
REM  well, you should go do that.

@echo off
set archiveName=UnoJoyWin-%date:~-10,2%-%date:~-7,2%-%date:~-4,4%.zip
if exist "UnoJoy/UnoJoy.h" (
  echo Found UnoJoy.
  echo.
  echo Collecting Windows files..
  REM Don't add in the OSX dfu-programmer files
  xcopy /i /y "UnoJoy\ATmega8u2Code\HexFiles\*.hex" "temp\UnoJoy\ATmega8u2Code\HexFiles"
  xcopy /i /y "UnoJoy\ATmega8u2Code\HexFiles\*.exe" "temp\UnoJoy\ATmega8u2Code\HexFiles"
  
  xcopy /i /y "UnoJoy/ATmega8u2Code" "temp/UnoJoy/ATmega8u2Code"
  xcopy /i /y /s "Drivers/Windows" "temp/UnoJoy/Drivers/Windows"
  xcopy /i /y /s "UnoJoy/Examples" "temp/UnoJoy/Examples"
  xcopy /i /y /s "UnoJoy/UnoJoyArduinoSample" "temp/UnoJoy/UnoJoyArduinoSample"
  
  REM Don't add in the OSX visualizer files
  xcopy /i /y "UnoJoy/UnoJoyProcessingVisualizer/data" "temp/UnoJoy/UnoJoyProcessingVisualizer/data"
  xcopy /i /y "UnoJoy/UnoJoyProcessingVisualizer/lib" "temp/UnoJoy/UnoJoyProcessingVisualizer/lib"
  xcopy /i /y "UnoJoy/UnoJoyProcessingVisualizer" "temp/UnoJoy/UnoJoyProcessingVisualizer"
  
  REM Only take the Windows root files
  copy /y "README.md" "temp/UnoJoy/README.txt" 
  copy /y "UnoJoy\TurnIntoAJoystick.bat" "temp/UnoJoy/TurnIntoAJoystick.bat" 
  copy /y "UnoJoy\TurnIntoAnArduino.bat" "temp/UnoJoy/TurnIntoAnArduino.bat" 
  copy /y "UnoJoy\UnoJoy.h" "temp/UnoJoy/UnoJoy.h" 
  echo "stuff"
  copy /y "Drivers\UnoJoyDriverInstaller.bat" "temp/UnoJoy/Drivers/UnoJoyDriverInstaller.bat"  
  
  REM Move up a directory
  cd temp/
  
  REM Zip up all the files
  7z a "archive.zip" "UnoJoy" -o/../..
  
  cd ..
  
  copy /y "temp\archive.zip" %archiveName%
  
  REM Delete the files, now that we've got them in a zip file
  rmdir temp /s /q
  
) else (
  echo Didn't find UnoJoy...
  echo Make sure you are running this file from the root of your UnoJoy directory
)
echo Press any key to exit...
pause > nul