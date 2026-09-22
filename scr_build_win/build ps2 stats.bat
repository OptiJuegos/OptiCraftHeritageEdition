@echo off
setlocal
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

"%CMAKE_EXE%" --preset ps2-release -DPS2_ENABLE_SOUND=ON -DPS2_ENABLE_PERSPECTIVE_TEXTURES=ON -DPS2_RENDER_STATS=ON -DPS2_ENABLE_VU1_TERRAIN=ON -DMC_LOG_LEVEL=2
"%CMAKE_EXE%" --build --preset ps2-release --parallel
pause
