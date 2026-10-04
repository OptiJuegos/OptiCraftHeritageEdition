@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"

REM Nintendo 3DS build (devkitARM + libctru)
REM
REM This script calls a NATIVE WINDOWS cmake from the candidate list below
REM instead of whatever `cmake` resolves to. devkitPro's installer puts its MSYS2
REM bin directory on the system PATH ahead of everything else, so a bare `cmake`
REM is the MSYS2 build even from cmd.exe. That one fails this project's CMake
REM compiler check with a misleading "the C compiler is broken" when the compiler
REM is perfectly fine -- verified on this machine.
REM
REM Same reason DEVKITPRO is dropped below when it holds a POSIX path: the
REM installer exports DEVKITPRO=/opt/devkitpro into the Windows environment too,
REM where it does not resolve.
REM
REM Usage:
REM   build 3ds.bat              configure + build + package bin/3ds/OptiCraft.cia
REM   build 3ds.bat clean        wipe the build directory first
REM   build 3ds.bat data         stage data/ into bin/3ds/sd/opticraft/
REM   build 3ds.bat clean data   wipe, rebuild, then stage
REM
REM The .cia is produced on EVERY build, not behind a keyword: it is the
REM installable form of the same ELF, so a run that stops at the .3dsx is not a
REM finished 3DS build. The CI job packages unconditionally for the same reason.
REM
REM Any argument that is not one of those keywords is forwarded verbatim to the
REM configure step, so build options go through this same entry point instead of
REM tempting you into a bare `cmake --preset` that picks up the MSYS2 cmake:
REM   build 3ds.bat -DMC_LOG_LEVEL=3
REM
REM devkitPro is located automatically at C:\devkitPro. Override it by setting
REM DEVKITPRO before running this script.

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
if defined CMAKE_EXE goto :cmakeFound

REM Last resort, and the one that needs the warning: whatever is on PATH may be
REM devkitPro's MSYS2 cmake (see the note at the top of this file) -- verified
REM to fail this project's compiler check.
for /f "delims=" %%M in ('where cmake 2^>nul') do if not defined CMAKE_EXE set "CMAKE_EXE=%%M"

:cmakeFound
if not defined CMAKE_EXE (
    echo ERROR: no native Windows cmake found and none on PATH.
    echo        Install CMake with the official installer ^(C:\Program Files\CMake^).
    exit /b 1
)
echo !CMAKE_EXE! | findstr /I /C:"msys2" /C:"devkitPro" >nul
if not errorlevel 1 (
    echo ERROR: the cmake found is devkitPro's MSYS2 build:
    echo        !CMAKE_EXE!
    echo        It fails this project's compiler check - see the note at the top.
    exit /b 1
)
echo Using cmake: !CMAKE_EXE!

REM devkitPro exports DEVKITPRO=/opt/devkitpro into the Windows environment; that
REM path only exists inside MSYS2. Drop it and let cmake/3ds_toolchain.cmake find
REM the real install instead of forwarding a value that cannot resolve.
if defined DEVKITPRO (
    if "!DEVKITPRO:~0,1!"=="/" (
        echo Ignoring MSYS2-style DEVKITPRO=!DEVKITPRO!
        set "DEVKITPRO="
    )
)

set PRESET=3ds-release
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
) else if /I "!ARG!"=="data" (
    set DODATA=1
) else (
    set "CMAKEARGS=!CMAKEARGS! !ARG!"
)
set "ALLARGS=!TAIL!"
goto parseargs
:parsedone

REM Staging is a plain directory copy, so it is done with xcopy rather than
REM through the cmake `3ds-data` target. Going through cmake would require a
REM configured build directory; this works before any build at all. The shape
REM matches that target: bin/3ds/sd is an SD-card root, so deploying is a copy
REM of its contents.
if defined DODATA (
    set "SDDATA=bin\3ds\sd\opticraft"
    echo Staging assets into !SDDATA! ...
    if not exist "!SDDATA!\data" mkdir "!SDDATA!\data"
    if exist "data\assets" (
        xcopy /E /I /Y /Q "data\assets" "!SDDATA!\data\assets"
        if errorlevel 1 goto :fail
    )
    if exist "data\resources_ps2" (
        xcopy /E /I /Y /Q "data\resources_ps2" "!SDDATA!\data\resources"
        if errorlevel 1 goto :fail
    )
    echo.
    echo Done. The game reads sdmc:/opticraft/assets.pak first, so remember to
    echo repack it with scripts\make_pak.py when you change the sources.
    echo Point Azahar's sdmc folder at:  %CD%\bin\3ds\sd
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

REM --- .cia packaging ------------------------------------------------------------
REM Straight from the ELF through the checked-in manifest, with no helper script
REM between them:
REM
REM     makerom -f cia -target t -exefslogo
REM             -rsf    resources\3ds_cia.rsf
REM             -elf    bin\3ds\OptiCraft.elf
REM             -icon   bin\3ds\OptiCraft.smdh
REM             -banner resources\3ds_banner.bnr
REM             -major 1 -minor 0 -micro 0
REM
REM makerom writes the exheader from the .rsf and lays out the code section from
REM the .elf itself, so there is no CXI conversion step and no cxitool. The
REM .smdh (built by cmake\3ds.cmake) re-supplies the HOME-menu title/publisher/
REM cover, so the identity the player reads stays in one place.
REM
REM The flags are the launch-tested posture (verified on a real Old 2DS,
REM 2026-09-28, with BOTH makerom v0.18.3 and v0.19.0): fully test-signed --
REM no -ignoresign, the flag was hiding whatever it was letting through -- a
REM real title version in the TMD, the ExeFS boot logo, and a HOME-menu
REM banner. The previously failing build differed in ALL of those at once;
REM which single one was lethal was never bisected apart, the posture ships
REM as a unit. makerom is PINNED to v0.18.3, matching CI -- the 3DS job
REM runs inside the devkitpro/devkitarm container (Ubuntu 22.04 base,
REM glibc 2.35) and v0.19.0's linux binary needs GLIBC_2.38; the Windows
REM build runs either way, but one version everywhere keeps the local and
REM CI .cia comparable. BOTH v0.18.3 and v0.19.0 were verified on hardware
REM under this posture, so the tool version is not the variable that
REM mattered. The
REM green text banner in resources\3ds_banner.bnr was made from resources\3ds_banner.png (256x128)
REM with bannertool; replacing the .png and re-running
REM   bannertool makebanner -i 3ds_banner.png -a <audio.wav> -o 3ds_banner.bnr
REM is the whole restyle.
REM
REM Runs at top level rather than inside an `if (...)` block: cmd cannot jump
REM *into* a parenthesized block -- the trailing `)` becomes a syntax error --
REM so every failure below exits to the top-level :fail label instead.
echo.
echo Packaging bin\3ds\OptiCraft.cia ...
if not exist "resources\3ds_cia.rsf" (
    echo ERROR: resources\3ds_cia.rsf not found.
    goto :fail
)
if not exist "resources\3ds_banner.bnr" (
    echo ERROR: resources\3ds_banner.bnr not found.
    echo        The HOME-menu banner is part of the launch-tested packaging
    echo        posture; see the note above for how to regenerate it.
    goto :fail
)
if not exist "bin\3ds\OptiCraft.elf" (
    echo ERROR: bin\3ds\OptiCraft.elf not found - the link produced nothing.
    goto :fail
)
if not exist "bin\3ds\OptiCraft.smdh" (
    echo ERROR: bin\3ds\OptiCraft.smdh not found - smdhtool did not run.
    echo        The .cia still needs it for the HOME-menu icon and title.
    goto :fail
)

call :findmakerom
if not defined MAKEROM call :fetchmakerom
if not defined MAKEROM goto :nomakerom

echo Using makerom: !MAKEROM!
"!MAKEROM!" -v -f cia -o "bin\3ds\OptiCraft.cia" -target t -exefslogo ^
    -rsf "resources\3ds_cia.rsf" ^
    -elf "bin\3ds\OptiCraft.elf" ^
    -icon "bin\3ds\OptiCraft.smdh" ^
    -banner "resources\3ds_banner.bnr" ^
    -major 1 -minor 0 -micro 0
if errorlevel 1 goto :fail
if not exist "bin\3ds\OptiCraft.cia" goto :fail

:done
echo.
echo Done.
echo   CIA         : bin\3ds\OptiCraft.cia   - install via FBI
echo   3DS/Azahar  : File ^> Open bin\3ds\OptiCraft.3dsx
echo   SD assets   : "build 3ds.bat data" first, then point Azahar's sdmc at bin\3ds\sd
pause
exit /b 0

:nomakerom
echo ERROR: makerom not found and it could not be downloaded.
echo        The .cia cannot be produced without it (3dstools ships 3dsxtool and
echo        smdhtool only, so makerom is a separate download):
echo          https://github.com/3DSGuy/Project_CTR/releases
echo        Put makerom.exe on PATH, or leave it in .tools\makerom\, then re-run.
echo        The .elf/.3dsx/.smdh above are still valid; Azahar can load the
echo        .3dsx directly.
goto :fail

REM --- locate makerom ------------------------------------------------------------
REM Not part of a devkitPro install: 3dstools ships 3dsxtool and smdhtool only.
REM Order is the repo-local copy, then PATH, then $DEVKITPRO\tools\bin.
:findmakerom
set "MAKEROM="
if exist "%~dp0.tools\makerom\makerom.exe" set "MAKEROM=%~dp0.tools\makerom\makerom.exe"
if defined MAKEROM exit /b 0
for /f "delims=" %%M in ('where makerom 2^>nul') do if not defined MAKEROM set "MAKEROM=%%M"
if defined MAKEROM exit /b 0
if exist "C:\devkitPro\tools\bin\makerom.exe" set "MAKEROM=C:\devkitPro\tools\bin\makerom.exe"
exit /b 0

:fetchmakerom
echo makerom not found - downloading v0.18.3 ...
if not exist "%~dp0.tools\makerom" mkdir "%~dp0.tools\makerom"
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$ErrorActionPreference='Stop'; $z='%~dp0.tools\makerom\makerom.zip';" ^
  "Invoke-WebRequest -Uri 'https://github.com/3DSGuy/Project_CTR/releases/download/makerom-v0.18.3/makerom-v0.18.3-win_x86_64.zip' -OutFile $z;" ^
  "Expand-Archive -Force -Path $z -DestinationPath '%~dp0.tools\makerom'; Remove-Item $z"
if exist "%~dp0.tools\makerom\makerom.exe" set "MAKEROM=%~dp0.tools\makerom\makerom.exe"
if defined MAKEROM echo makerom ready: !MAKEROM!
exit /b 0

:fail
echo.
echo BUILD FAILED
pause
exit /b 1
