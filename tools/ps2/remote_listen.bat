@echo off
setlocal
if "%~1"=="" (
    echo Usage: remote_listen.bat ^<PS2_IP^>
    exit /b 2
)
set "PS2_IP=%~1"
where ps2client.exe >nul 2>nul
if errorlevel 1 (
    echo ps2client.exe was not found in PATH.
    echo Add the ps2dev bin directory to PATH or run this from a ps2dev shell.
    exit /b 1
)
echo Listening to ps2link at %PS2_IP%...
ps2client.exe -h %PS2_IP% listen
