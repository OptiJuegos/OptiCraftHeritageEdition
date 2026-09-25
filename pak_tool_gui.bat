@echo off
setlocal
cd /d "%~dp0"
title OptiCraft - Herramienta de Assets.pak (GUI)

REM Intenta ejecutar con pythonw (sin ventana de consola) o con python standard
where pythonw >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    start "" pythonw "%~dp0scripts\pak_tool_gui.py" %*
) else (
    start "" python "%~dp0scripts\pak_tool_gui.py" %*
)
