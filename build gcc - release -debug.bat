@echo off
REM This script deliberately calls a NATIVE WINDOWS cmake by full path instead of
REM whatever `cmake` resolves to. devkitPro's installer puts its MSYS2 bin
REM directory on the system PATH ahead of everything else, so a bare `cmake` is
REM the MSYS2 build even from cmd.exe. That one emits POSIX paths (/d/MC/...)
REM into build.ninja, which the repository's native ninja.exe reads as
REM \d\MC\... -- and the build dies inside CMake's compiler check with a
REM misleading "the C compiler is broken" when the compiler is perfectly fine.
REM See build wii.bat for the same fix applied to the Wii preset.
setlocal enabledelayedexpansion
cd /d "%~dp0"

set "CMAKE_EXE="
for %%C in (
    "%ProgramFiles%\CMake\bin\cmake.exe"
    "%ProgramFiles(x86)%\CMake\bin\cmake.exe"
    "%ProgramFiles%\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    "%ProgramFiles%\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
) do (
    if not defined CMAKE_EXE if exist %%C set "CMAKE_EXE=%%~C"
)
if not defined CMAKE_EXE (
    echo WARNING: no native Windows cmake found; falling back to whatever is on PATH.
    echo          If that is devkitPro's MSYS2 cmake the build will fail in the
    echo          compiler check - see the note at the top of this file.
    set "CMAKE_EXE=cmake"
)
echo Using cmake: !CMAKE_EXE!

"!CMAKE_EXE!" --preset gcc-release -DMC_LOG_LEVEL=2
if errorlevel 1 goto :fail

"!CMAKE_EXE!" --build --preset gcc-release --parallel 
if errorlevel 1 goto :fail

pause
exit /b 0

:fail
echo.
echo BUILD FAILED
pause
exit /b 1
