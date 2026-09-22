@echo off
setlocal
cd /d "%~dp0"

REM Nintendo Wii build (devkitPPC + libogc). See src/wii/README_wii.md for how to
REM install the toolchain.
REM
REM This script deliberately calls a NATIVE WINDOWS cmake by full path instead of
REM whatever `cmake` resolves to. devkitPro's installer puts its MSYS2 bin
REM directory on the system PATH ahead of everything else, so a bare `cmake` is
REM the MSYS2 build even from cmd.exe. That one emits POSIX paths (/d/MC/...)
REM into build.ninja, which the repository's native ninja.exe reads as
REM \d\MC\... -- and the build dies inside CMake's compiler check with a
REM misleading "the C compiler is broken" when the compiler is perfectly fine.
REM
REM Same reason DEVKITPRO is dropped below when it holds a POSIX path: the
REM installer exports DEVKITPRO=/opt/devkitpro into the Windows environment too,
REM where it does not resolve.
REM
REM Usage:
REM   build wii.bat              incremental bring-up build (src/wii/main_wii.cpp)
REM   build wii.bat game         build the full game instead of the bring-up test
REM   build wii.bat clean        wipe the bring-up build dir first
REM   build wii.bat clean game   wipe the full-game build dir first
REM   build wii.bat data         stage data/ into bin/wii/sd/apps/BetaPlusPlus/data
REM
REM Any argument that is not one of those keywords is forwarded verbatim to the
REM configure step, so build options go through this same entry point instead of
REM tempting you into a bare `cmake --preset` that picks up the MSYS2 cmake:
REM   build wii.bat game -DWII_MODEL_DISPLAY_LISTS=ON
REM
REM devkitPro is located automatically at C:\devkitPro. Override it by setting
REM DEVKITPRO before running this script.

setlocal enabledelayedexpansion

REM --- pick a native Windows cmake ---------------------------------------------
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

REM devkitPro exports DEVKITPRO=/opt/devkitpro into the Windows environment; that
REM path only exists inside MSYS2. Drop it and let cmake/wii_toolchain.cmake find
REM the real install instead of forwarding a value that cannot resolve.
if defined DEVKITPRO (
    if "!DEVKITPRO:~0,1!"=="/" (
        echo Ignoring MSYS2-style DEVKITPRO=!DEVKITPRO!
        set "DEVKITPRO="
    )
)

set PRESET=wii-release
set DOCLEAN=
set DODATA=
set "CMAKEARGS="

REM Keywords are recognised in any position; everything else is collected and
REM passed on to the configure call. See the usage note at the top.
REM
REM This walks the raw command line with `for /f` rather than %1 + shift, because
REM cmd tokenises %1..%9 on `=` as well as on whitespace: -DFOO=ON arrives as two
REM parameters, and rejoining them with a space turns the second into cmake's
REM source-directory argument ("Could not read presets from .../ON"). `for /f`
REM only splits on space and tab, so `=` survives.
set "ALLARGS=%*"
:parseargs
if not defined ALLARGS goto parsedone
set "ARG="
set "TAIL="
for /f "tokens=1,*" %%A in ("!ALLARGS!") do (
    set "ARG=%%A"
    set "TAIL=%%B"
)
if /I "!ARG!"=="clean" (
    set DOCLEAN=1
) else if /I "!ARG!"=="game" (
    set PRESET=wii-release
) else if /I "!ARG!"=="data" (
    set DODATA=1
) else (
    set "CMAKEARGS=!CMAKEARGS! !ARG!"
)
set "ALLARGS=!TAIL!"
goto parseargs
:parsedone

REM Staging is a plain directory copy, so it is done with xcopy rather than
REM through the cmake `wii-data` target. Going through cmake would require a
REM configured build directory and would have to know *which* preset was
REM configured (wii-release or wii-game); this works before any build at all.
if defined DODATA (
    set "SDDATA=bin\wii\sd\apps\BetaPlusPlus\data"
    echo Staging assets into !SDDATA! ...
    if not exist "!SDDATA!" mkdir "!SDDATA!"
    xcopy /E /I /Y /Q "data\assets"    "!SDDATA!\assets"
    if errorlevel 1 goto :fail
    if exist "data\resources" (
        xcopy /E /I /Y /Q "data\resources" "!SDDATA!\resources"
        if errorlevel 1 goto :fail
    )
    echo.
    echo Done. Point Dolphin's SD Sync Folder at:
    echo     %CD%\bin\wii\sd
    pause
    exit /b 0
)

REM A failed configure leaves a cache holding the compiler paths it resolved. If
REM those came from a different shell (see the MSYS2 note above), reusing them
REM keeps reproducing the original failure, so wiping is the fix rather than a
REM precaution.
if defined DOCLEAN (
    if exist "build\!PRESET!" (
        echo Cleaning build\!PRESET! ...
        rmdir /s /q "build\!PRESET!"
    )
)

echo Configuring preset !PRESET! ...
if defined CMAKEARGS echo Extra cmake options:!CMAKEARGS!
if defined DEVKITPRO (
    echo Using DEVKITPRO=!DEVKITPRO!
    "!CMAKE_EXE!" --preset !PRESET! -DDEVKITPRO=!DEVKITPRO!!CMAKEARGS!
) else (
    "!CMAKE_EXE!" --preset !PRESET!!CMAKEARGS!
)
if errorlevel 1 goto :fail

"!CMAKE_EXE!" --build --preset !PRESET! --parallel
if errorlevel 1 goto :fail

echo.
echo Done. Deploy: copy bin\wii\sd\* to the root of the SD card.
echo        Dolphin: open bin\wii\BetaPlusPlus.elf directly.
pause
exit /b 0

:fail
echo.
echo BUILD FAILED
pause
exit /b 1
