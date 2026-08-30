@echo off
setlocal

powershell -ExecutionPolicy Bypass -File "%~dp0tools\convert_ps2_audio.ps1" ^
  -InputPath "data\resources" ^
  -OutputPath "data\resources_ps2" ^
  -Mode Both ^
  -SampleRate 22050 ^
  -Channels mono ^
  -SkipFailed ^
  -Overwrite

if errorlevel 1 (
  echo.
  echo PS2 audio conversion failed.
  exit /b 1
)

echo.
echo PS2 audio conversion complete.
