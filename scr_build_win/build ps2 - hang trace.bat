@echo off
setlocal
cd /d "%~dp0"

REM Diagnostic build for a console that FREEZES rather than crashes.
REM
REM Differences from "build ps2.bat":
REM   MC_LOG_LEVEL=3      turns on MC_LOG_TRACE, which emits the per-frame
REM                       breadcrumbs in Display_ps2.cpp (gs queue exec ->
REM                       present -> presented). The last one written names the
REM                       call the console stopped inside.
REM   PS2_RENDER_STATS=ON adds the draw/depth/VU1 counter dumps to the periodic
REM                       FRAME line.
REM
REM Log writes are synced per line on PS2 (MC_LOG_SYNC_WRITES in Log.h), so the
REM tail of data/debug.log survives a hang instead of dying in the FILE buffer.
REM That means one filesystem sync per line: EXPECT THIS BUILD TO BE SLOW. It is
REM for reading debug.log after a freeze, not for playing.
REM
REM Sound stays OFF to match the build being debugged.

if /I "%~1"=="clean" (
    if exist "build\ps2-release" rmdir /s /q "build\ps2-release"
)

set "CMAKE_EXE=C:\Program Files\CMake\bin\cmake.exe"
if not exist "%CMAKE_EXE%" (
    echo ERROR: Native Windows CMake was not found at "%CMAKE_EXE%".
    exit /b 1
)

"%CMAKE_EXE%" --preset ps2-release -DPS2_ENABLE_SOUND=ON -DPS2_ENABLE_PERSPECTIVE_TEXTURES=ON -DPS2_RENDER_STATS=ON -DPS2_ENABLE_VU1_TERRAIN=ON -DMC_LOG_LEVEL=3
if errorlevel 1 exit /b 1
"%CMAKE_EXE%" --build --preset ps2-release --parallel 10
if errorlevel 1 exit /b 1
pause
