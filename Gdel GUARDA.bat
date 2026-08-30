@echo off
cd /d "%~dp0"

if exist "src" rmdir /s /q "src"
if exist "srcorigjava" rmdir /s /q "srcorigjava"
if exist "cmake" rmdir /s /q "cmake"

if exist "CMakeLists.txt" del /f /q "CMakeLists.txt"
if exist "CMakePresets.json" del /f /q "CMakePresets.json"
if exist "AGENT.MD" del /f /q "AGENT.MD"
if exist "CODESTYLE.MD" del /f /q "CODESTYLE.MD"

echo Listo.
pause