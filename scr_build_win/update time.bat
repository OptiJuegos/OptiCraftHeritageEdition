@echo off
setlocal
cd /d "%~dp0"

REM Repair file timestamps that sit ahead of the wall clock.
REM
REM Ninja regenerates build.ninja whenever one of its CMake inputs (CMakeLists.txt,
REM cmake/*.cmake, the toolchain file) is newer than build.ninja itself. When a bulk
REM copy or an archive extraction stamps the tree with UTC times while the machine
REM runs on a negative UTC offset, every file lands hours in the future. CMake then
REM reconfigures, rewrites build.ninja with the current time -- still older than the
REM sources -- and ninja tries again, so the build spins on
REM     [0/2] Re-checking globbed directories...
REM     [1/2] Re-running CMake...
REM forever and never reaches a single compile. Restamping the offending files to
REM the current time breaks the loop.
REM
REM Only files dated in the future are restamped, so a run on a healthy tree changes
REM nothing and does not force a full rebuild. Pass "all" for the old unconditional
REM behaviour.
REM
REM Usage:
REM   update time.bat        restamp the files dated in the future
REM   update time.bat all    restamp every file in the tree
REM
REM .git is never touched: git keeps its own stat cache in there and restamping it
REM only makes the next git command re-read the whole index.

set "UPDATE_TIME_MODE=%~1"

REM The script is assembled line by line because a .bat cannot host a multi-line
REM PowerShell block. Single quotes only -- a double quote would close the SET.
set "PS=$now = Get-Date;"
set "PS=%PS% $limit = $now.AddSeconds(2);"
set "PS=%PS% $all = ($env:UPDATE_TIME_MODE -eq 'all');"
set "PS=%PS% $targets = @(Get-ChildItem -Recurse -File -Force | Where-Object { $_.FullName -notlike '*\.git\*' -and ($all -or $_.LastWriteTime -gt $limit) });"
set "PS=%PS% if ($targets.Count -eq 0) { Write-Host 'Timestamps are sane; nothing to restamp.'; exit 0 };"
set "PS=%PS% $fixed = 0; $failed = 0;"
set "PS=%PS% foreach ($f in $targets) { try { $f.LastWriteTime = $now; $fixed++ } catch { $failed++ } };"
set "PS=%PS% Write-Host ('Restamped ' + $fixed + ' file(s) to ' + $now.ToString('yyyy-MM-dd HH:mm:ss') + '.');"
set "PS=%PS% if ($failed -gt 0) { Write-Host ('WARNING: ' + $failed + ' file(s) could not be restamped (read-only or locked).') };"
set "PS=%PS% $stale = @($targets | Where-Object { $_.FullName -like '*\build\*' }).Count;"
set "PS=%PS% if ($stale -gt 0) { Write-Host ''; Write-Host ('WARNING: ' + $stale + ' of them live under build\, so incremental state there is'); Write-Host '         unreliable. Reconfigure from scratch, e.g. build wii.bat clean game.' };"

powershell -NoProfile -ExecutionPolicy Bypass -Command "%PS%"
set "RC=%ERRORLEVEL%"

endlocal & exit /b %RC%
