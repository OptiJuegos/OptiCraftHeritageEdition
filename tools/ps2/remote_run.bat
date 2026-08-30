@echo off
setlocal
if "%~1"=="" (
    echo Usage: remote_run.bat ^<PS2_IP^> [ELF]
    exit /b 2
)
set "PS2_IP=%~1"
set "ELF=%~2"
if "%ELF%"=="" set "ELF=%~dp0..\..\bin\ps2\BetaPlusPlus.elf"
where ps2client.exe >nul 2>nul
if errorlevel 1 (
    echo ps2client.exe was not found in PATH.
    exit /b 1
)
if not exist "%ELF%" (
    echo ELF not found: %ELF%
    exit /b 1
)
for %%I in ("%ELF%") do set "ELF_DIR=%%~dpI"& set "ELF_NAME=%%~nxI"
pushd "%ELF_DIR%"
echo Executing %ELF_NAME% on %PS2_IP% through ps2link...
ps2client.exe -h %PS2_IP% execee host:%ELF_NAME%
set "RC=%ERRORLEVEL%"
popd
exit /b %RC%
