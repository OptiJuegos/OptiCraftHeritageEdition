@echo off
cd /d "%~dp0"
del "mcbeta.zip"
7z a mcbeta.zip "src" "srcorigjava" "cmake" "CMakeLists.txt" "CMakePresets.json" "AGENT.MD" "CODESTYLE.MD"