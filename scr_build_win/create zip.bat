@echo off
cd /d "%~dp0"
del "mcrelease.zip"
7z a mcrelease.zip "src" "srcorigjava" "cmake" "CMakeLists.txt" "CMakePresets.json" "AGENT.MD" "CODESTYLE.MD"