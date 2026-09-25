@echo off
setlocal
cd /d "%~dp0"
title OptiCraft Heritage - Desempaquetador de Texturas y Assets

python "%~dp0scripts\unpack_pak.py" %*
echo.
pause
