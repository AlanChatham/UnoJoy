@echo off
if not exist Drivers/dpinst-amd64.exe (
echo.
echo The program used install the drivers could not be found...
echo Did you move this .bat file?  Or delete something in the Drivers folder?
echo Press any key to exit...
pause > nul
goto EXIT
)
cd Drivers
dpinst-amd64.exe
:EXIT