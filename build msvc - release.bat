@echo off
setlocal
cd /d "%~dp0"

REM only when CMake/source-list/toolchain changes leave stale objects behind.
if /I "%~1"=="clean" (
    if exist "build\ps2-release" rmdir /s /q "build\ps2-release"
)

set "CMAKE_EXE=C:\Program Files\CMake\bin\cmake.exe"
if not exist "%CMAKE_EXE%" (
    echo ERROR: Native Windows CMake was not found at "%CMAKE_EXE%".
    exit /b 1
)


"%CMAKE_EXE%" -S . -B build
"%CMAKE_EXE%" --build build --config release --target BetaPlusPlus --parallel
pause
