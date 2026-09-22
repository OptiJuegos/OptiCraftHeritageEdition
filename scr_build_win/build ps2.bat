@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"

REM PS2 build changes often add/remove generated objects and compatibility shims.
REM Default to incremental builds so small PS2 iteration changes do not rebuild
REM the whole project. Use:
REM   build ps2.bat clean
REM only when CMake/source-list/toolchain changes leave stale objects behind.
if /I "%~1"=="clean" (
    if exist "build\ps2-release" rmdir /s /q "build\ps2-release"
)

set "CMAKE_EXE=C:\Program Files\CMake\bin\cmake.exe"
if not exist "%CMAKE_EXE%" (
    echo ERROR: Native Windows CMake was not found at "%CMAKE_EXE%".
    exit /b 1
)

REM Staging is a plain directory copy, so it is done with xcopy rather than
REM through the cmake `ps2-data` target: this works before any build at all,
REM without a configured build directory. bin\ps2\usb is shaped like the root of
REM the pendrive -- copy its contents to the drive.
REM
REM data\resources_ps2 is staged as data\resources because that is the name the
REM asset keys use. newsound\ambient and sound\loops are dropped for SPU2 space;
REM see the ps2-data target in cmake\ps2.cmake.
if /I "%~1"=="data" (
    set "USBDATA=bin\ps2\usb\MCBETA\data"
    echo Staging assets into !USBDATA! ...
    if not exist "!USBDATA!" mkdir "!USBDATA!"
    xcopy /E /I /Y /Q "data\assets"        "!USBDATA!\assets"
    if errorlevel 1 exit /b 1
    xcopy /E /I /Y /Q "data\startup"       "!USBDATA!\startup"
    if errorlevel 1 exit /b 1
    REM Wiped rather than merged: xcopy would leave behind whatever a previous
    REM staging put there, and a stale desktop data\resources (OGG) mixed into
    REM the converted ADPCM tree is 20 MB of files the PS2 mixer cannot play.
    if exist "!USBDATA!\resources" rmdir /s /q "!USBDATA!\resources"
    xcopy /E /I /Y /Q "data\resources_ps2" "!USBDATA!\resources"
    if errorlevel 1 exit /b 1
    if exist "!USBDATA!\resources\newsound\ambient" rmdir /s /q "!USBDATA!\resources\newsound\ambient"
    if exist "!USBDATA!\resources\sound\loops"      rmdir /s /q "!USBDATA!\resources\sound\loops"
    REM The runtime never lists directories: it reads this index instead.
    "%CMAKE_EXE%" -D "INDEX_ROOT=%CD:\=/%/bin/ps2/usb/MCBETA/data/resources" -P "cmake/ps2_resource_index.cmake"
    if errorlevel 1 exit /b 1
    if not exist "!USBDATA!\irx" mkdir "!USBDATA!\irx"
    copy /Y "psdevwindows\ps2sdk\iop\irx\audsrv.irx" "!USBDATA!\irx\audsrv.irx" >nul
    echo.
    echo Done. Copy the contents of bin\ps2\usb to the root of the USB drive:
    echo     %CD%\bin\ps2\usb
    pause
    exit /b 0
)

"%CMAKE_EXE%" --preset ps2-release -DPS2_ENABLE_SOUND=OFF -DPS2_ENABLE_PERSPECTIVE_TEXTURES=ON -DPS2_RENDER_STATS=OFF -DPS2_ENABLE_VU1_TERRAIN=ON -DMC_LOG_LEVEL=0
if errorlevel 1 exit /b 1
"%CMAKE_EXE%" --build --preset ps2-release --parallel
if errorlevel 1 exit /b 1
pause
