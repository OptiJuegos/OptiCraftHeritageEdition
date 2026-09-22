@echo off
setlocal
cd /d "%~dp0"

REM ROOT-ONLY version. Keep this file in the project root.
if not exist "CMakeLists.txt" (
    echo ERROR: this script must be kept in the project root.
    exit /b 1
)

if not exist "src\" (
    echo ERROR: src\ was not found. Refusing to continue.
    exit /b 1
)

echo DANGER: this will permanently delete:
echo   src\
echo   srcorigjava\
echo   cmake\
echo   CMakeLists.txt
echo   CMakePresets.json
echo   AGENT.MD
echo   CODESTYLE.MD
echo.
echo Project root:
echo   %CD%
echo.
set /p "CONFIRM=Type DELETE SOURCE TREE to continue: "

if not "%CONFIRM%"=="DELETE SOURCE TREE" (
    echo Cancelled.
    exit /b 1
)

if exist "src\" rmdir /s /q "src"
if exist "srcorigjava\" rmdir /s /q "srcorigjava"
if exist "cmake\" rmdir /s /q "cmake"

if exist "CMakeLists.txt" del /f /q "CMakeLists.txt"
if exist "CMakePresets.json" del /f /q "CMakePresets.json"
if exist "AGENT.MD" del /f /q "AGENT.MD"
if exist "CODESTYLE.MD" del /f /q "CODESTYLE.MD"

echo Deleted.
exit /b 0
