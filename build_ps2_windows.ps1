# MC:PS2 / OptiCraft - automatic Windows build launcher.
# Option 0 performs the standard PS2 build.
# Every other .bat in scr_build_win is discovered dynamically; before executing
# it, this launcher checks/prepares the dependencies appropriate to that script.

[CmdletBinding()]
param(
    [switch]$Clean,
    [switch]$Yes
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$LegacyDir = Join-Path $Root "scr_build_win"
$BuildDir = Join-Path $Root "build\ps2"
$AppDir = Join-Path $Root "bin\ps2\usb\MCBETA"
$Elf = Join-Path $AppDir "OptiCraft.elf"
$Pak = Join-Path $AppDir "assets.pak"
$Map = Join-Path $BuildDir "OptiCraft.map"

function Fail([string]$Message) {
    throw $Message
}

function Ask-YesNo([string]$Message) {
    if ($Yes) {
        Write-Host "$Message [auto: yes]"
        return $true
    }

    $answer = Read-Host "$Message [Y/n]"
    if ([string]::IsNullOrWhiteSpace($answer)) { return $true }
    return @("y", "yes", "s", "sim") -contains $answer.Trim().ToLowerInvariant()
}

function Refresh-Path {
    $machine = [Environment]::GetEnvironmentVariable("Path", "Machine")
    $user = [Environment]::GetEnvironmentVariable("Path", "User")
    $parts = @($machine, $user, $env:Path) | Where-Object { -not [string]::IsNullOrWhiteSpace($_) }
    $env:Path = $parts -join ";"
}

function Require-Winget {
    $winget = Get-Command winget.exe -ErrorAction SilentlyContinue
    if (-not $winget) {
        Fail "winget was not found. Install 'App Installer' from Microsoft or install the missing dependency manually."
    }
    return $winget.Source
}

function Install-WingetPackage(
    [string]$Id,
    [string[]]$ExtraArguments = @()
) {
    $winget = Require-Winget
    Write-Host "Installing $Id ..."

    $args = @(
        "install",
        "--id", $Id,
        "-e",
        "--accept-package-agreements",
        "--accept-source-agreements"
    ) + $ExtraArguments

    & $winget @args
    if ($LASTEXITCODE -ne 0) {
        Fail "winget failed to install $Id (exit code $LASTEXITCODE)."
    }

    Refresh-Path
}

function Find-CMake {
    # Prefer native Windows CMake. devkitPro/MSYS2 can put a POSIX CMake first
    # on PATH, which the original OptiJuegos scripts deliberately avoid.
    $known = @(
        (Join-Path $env:ProgramFiles "CMake\bin\cmake.exe"),
        (Join-Path ${env:ProgramFiles(x86)} "CMake\bin\cmake.exe"),
        (Join-Path $env:ProgramFiles "Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"),
        (Join-Path $env:ProgramFiles "Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"),
        (Join-Path $env:ProgramFiles "Microsoft Visual Studio\2022\Enterprise\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe")
    )

    foreach ($candidate in $known) {
        if ($candidate -and (Test-Path -LiteralPath $candidate)) {
            return $candidate
        }
    }

    $cmd = Get-Command cmake.exe -ErrorAction SilentlyContinue
    if ($cmd -and $cmd.Source -notmatch '\\devkitPro\\|\\msys2\\|\\usr\\bin\\') {
        return $cmd.Source
    }

    return $null
}

function Test-CMakeVersion([string]$CMake) {
    if (-not $CMake) { return $false }

    try {
        $line = (& $CMake --version | Select-Object -First 1)
        if ($line -match 'cmake version\s+(\d+)\.(\d+)') {
            $major = [int]$Matches[1]
            $minor = [int]$Matches[2]
            return ($major -gt 3 -or ($major -eq 3 -and $minor -ge 21))
        }
    } catch {}

    return $false
}

function Ensure-NativeCMake {
    $cmake = Find-CMake
    if (Test-CMakeVersion $cmake) {
        return $cmake
    }

    if (-not (Ask-YesNo "Native Windows CMake 3.21+ is missing. Install CMake now?")) {
        Fail "Native Windows CMake 3.21+ is required."
    }

    Install-WingetPackage "Kitware.CMake"
    $cmake = Find-CMake

    if (-not (Test-CMakeVersion $cmake)) {
        Fail "Native Windows CMake 3.21+ was not found after installation."
    }

    return $cmake
}

function Find-Ninja {
    $bundled = Join-Path $Root "ninja.exe"
    if (Test-Path -LiteralPath $bundled) { return $bundled }

    $cmd = Get-Command ninja.exe -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }

    foreach ($candidate in @(
        (Join-Path $env:ProgramFiles "Ninja\ninja.exe"),
        (Join-Path $env:LOCALAPPDATA "Microsoft\WinGet\Links\ninja.exe")
    )) {
        if (Test-Path -LiteralPath $candidate) { return $candidate }
    }

    return $null
}

function Ensure-RootNinja {
    $rootNinja = Join-Path $Root "ninja.exe"
    if (Test-Path -LiteralPath $rootNinja) {
        return $rootNinja
    }

    $ninja = Find-Ninja
    if (-not $ninja) {
        if (-not (Ask-YesNo "Ninja is missing. Install Ninja now?")) {
            Fail "Ninja is required."
        }

        Install-WingetPackage "Ninja-build.Ninja"
        $ninja = Find-Ninja
    }

    if (-not $ninja) {
        Fail "Ninja was not found after installation."
    }

    Copy-Item -LiteralPath $ninja -Destination $rootNinja -Force
    return $rootNinja
}

function Get-GccPresetNinjaPath {
    $presetPath = Join-Path $Root "CMakePresets.json"
    if (-not (Test-Path -LiteralPath $presetPath)) {
        return $null
    }

    try {
        $json = Get-Content -LiteralPath $presetPath -Raw | ConvertFrom-Json
        $preset = $json.configurePresets |
            Where-Object { $_.name -eq "gcc-debug" } |
            Select-Object -First 1

        if ($preset -and $preset.cacheVariables -and $preset.cacheVariables.CMAKE_MAKE_PROGRAM) {
            $value = $preset.cacheVariables.CMAKE_MAKE_PROGRAM
            if ($value -isnot [string] -and $value.value) {
                $value = $value.value
            }
            if ($value -is [string]) {
                return ($value -replace '/', '\')
            }
        }
    } catch {
        Write-Warning "Could not parse CMakePresets.json to find the GCC Ninja path: $($_.Exception.Message)"
    }

    return $null
}

function Invoke-ElevatedPowerShell([string]$Command) {
    $encoded = [Convert]::ToBase64String([Text.Encoding]::Unicode.GetBytes($Command))
    $process = Start-Process -FilePath "powershell.exe" `
        -Verb RunAs `
        -Wait `
        -PassThru `
        -ArgumentList @("-NoProfile", "-ExecutionPolicy", "Bypass", "-EncodedCommand", $encoded)

    if ($process.ExitCode -ne 0) {
        Fail "The elevated PowerShell command failed with exit code $($process.ExitCode)."
    }
}

function Ensure-GccPresetNinja {
    $ninja = Ensure-RootNinja
    $expected = Get-GccPresetNinjaPath

    if ([string]::IsNullOrWhiteSpace($expected)) {
        return
    }

    if ($expected -match '^\$\{sourceDir\}') {
        $expected = $expected -replace '^\$\{sourceDir\}', [Regex]::Escape($Root)
    }

    if (Test-Path -LiteralPath $expected) {
        return
    }

    Write-Host ""
    Write-Host "The GCC preset expects Ninja at:"
    Write-Host "  $expected"
    Write-Host "The project already has Ninja at:"
    Write-Host "  $ninja"

    if (-not (Ask-YesNo "Create a compatibility copy of ninja.exe at the preset path?")) {
        Fail "The selected GCC preset requires Ninja at '$expected'."
    }

    $parent = Split-Path -Parent $expected
    try {
        New-Item -ItemType Directory -Force -Path $parent | Out-Null
        Copy-Item -LiteralPath $ninja -Destination $expected -Force
    } catch {
        Write-Host "Administrator permission is required for that path."
        $src = $ninja.Replace("'", "''")
        $dst = $expected.Replace("'", "''")
        $dir = $parent.Replace("'", "''")
        Invoke-ElevatedPowerShell "New-Item -ItemType Directory -Force -Path '$dir' | Out-Null; Copy-Item -LiteralPath '$src' -Destination '$dst' -Force"
    }

    if (-not (Test-Path -LiteralPath $expected)) {
        Fail "Could not create the Ninja compatibility copy at '$expected'."
    }
}

function Find-Python {
    $py = Get-Command py.exe -ErrorAction SilentlyContinue
    if ($py) {
        try {
            & $py.Source -3 --version *> $null
            if ($LASTEXITCODE -eq 0) {
                return [pscustomobject]@{ Exe = $py.Source; PrefixArgs = @("-3") }
            }
        } catch {}
    }

    foreach ($name in @("python.exe", "python3.exe")) {
        $cmd = Get-Command $name -ErrorAction SilentlyContinue
        if ($cmd) {
            try {
                & $cmd.Source --version *> $null
                if ($LASTEXITCODE -eq 0) {
                    return [pscustomobject]@{ Exe = $cmd.Source; PrefixArgs = @() }
                }
            } catch {}
        }
    }

    $base = Join-Path $env:LOCALAPPDATA "Programs\Python"
    if (Test-Path -LiteralPath $base) {
        $candidate = Get-ChildItem $base -Directory -Filter "Python*" -ErrorAction SilentlyContinue |
            Sort-Object Name -Descending |
            ForEach-Object { Join-Path $_.FullName "python.exe" } |
            Where-Object { Test-Path -LiteralPath $_ } |
            Select-Object -First 1

        if ($candidate) {
            return [pscustomobject]@{ Exe = $candidate; PrefixArgs = @() }
        }
    }

    return $null
}

function Install-PythonWithWinget {
    foreach ($id in @(
        "Python.Python.3.14",
        "Python.Python.3.13",
        "Python.Python.3.12",
        "Python.Python.3.11"
    )) {
        try {
            Install-WingetPackage $id
            return
        } catch {
            Write-Warning "Could not install $id; trying another supported Python package."
        }
    }

    Fail "Could not install Python 3 through winget."
}

function Ensure-Python {
    $python = Find-Python
    if ($python) { return $python }

    if (-not (Ask-YesNo "Python 3 is missing. Install Python now?")) {
        Fail "Python 3 is required for the standard PS2 assets.pak step."
    }

    Install-PythonWithWinget
    $python = Find-Python
    if (-not $python) {
        Fail "Python 3 was not found after installation."
    }

    return $python
}

function Test-Ps2Dev([string]$Path) {
    if ([string]::IsNullOrWhiteSpace($Path)) { return $false }

    $checks = @(
        "ee\bin\mips64r5900el-ps2-elf-g++.exe",
        "ps2sdk\iop\irx\audsrv.irx",
        "ps2sdk\iop\irx\ps2dev9.irx",
        "ps2sdk\iop\irx\netman.irx",
        "ps2sdk\iop\irx\smap.irx",
        "gsKit"
    )

    foreach ($relative in $checks) {
        if (-not (Test-Path -LiteralPath (Join-Path $Path $relative))) {
            return $false
        }
    }

    return $true
}

function Find-Tar {
    $cmd = Get-Command tar.exe -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }

    $systemTar = Join-Path $env:SystemRoot "System32\tar.exe"
    if (Test-Path -LiteralPath $systemTar) { return $systemTar }

    $gitTar = Join-Path $env:ProgramFiles "Git\usr\bin\tar.exe"
    if (Test-Path -LiteralPath $gitTar) { return $gitTar }

    return $null
}

function Install-Ps2Dev([string]$Destination) {
    $tar = Find-Tar

    if (-not $tar) {
        if (Ask-YesNo "tar.exe is missing. Install Git for Windows (includes tar) now?") {
            Install-WingetPackage "Git.Git"
            $tar = Find-Tar
        }
    }

    if (-not $tar) {
        Fail "tar.exe is required to extract PS2DEV."
    }

    if (Test-Path -LiteralPath $Destination) {
        $items = @(Get-ChildItem -Force -LiteralPath $Destination -ErrorAction SilentlyContinue)
        if ($items.Count -gt 0) {
            if (-not (Ask-YesNo "The incomplete PS2DEV directory already contains files. Remove it before reinstalling?")) {
                Fail "Cannot safely install over an existing incomplete PS2DEV directory."
            }
            Remove-Item -Recurse -Force -LiteralPath $Destination
        }
    }

    New-Item -ItemType Directory -Force -Path $Destination | Out-Null

    Write-Host "Finding the official Windows PS2DEV development package..."
    $headers = @{ "User-Agent" = "MCPS2-Windows-Build-Script" }
    $release = Invoke-RestMethod -Headers $headers -Uri "https://api.github.com/repos/ps2dev/ps2dev/releases/tags/latest"
    $asset = $release.assets |
        Where-Object { $_.name -match '^ps2dev-windows.*\.tar\.gz$' } |
        Select-Object -First 1

    if (-not $asset) {
        Fail "The official PS2DEV latest release does not contain a Windows .tar.gz asset."
    }

    $archive = Join-Path $env:TEMP $asset.name
    Write-Host "Downloading $($asset.name) ..."
    Invoke-WebRequest -Headers $headers -Uri $asset.browser_download_url -OutFile $archive

    Write-Host "Extracting PS2DEV into $Destination ..."
    & $tar -xzf $archive --strip-components=1 -C $Destination
    if ($LASTEXITCODE -ne 0) {
        Fail "Failed to extract PS2DEV."
    }

    Remove-Item -Force -LiteralPath $archive -ErrorAction SilentlyContinue
}

function Ensure-ProjectPs2Dev {
    # The OptiJuegos ps2-release preset explicitly points to
    # ${sourceDir}/psdevwindows, so legacy PS2 .bat files require this path.
    $projectPs2Dev = Join-Path $Root "psdevwindows"
    if (Test-Ps2Dev $projectPs2Dev) {
        return $projectPs2Dev
    }

    $external = $null
    if ($env:PS2DEV -and (Test-Ps2Dev $env:PS2DEV)) {
        $external = $env:PS2DEV
    } elseif (Test-Ps2Dev "C:\ps2dev") {
        $external = "C:\ps2dev"
    }

    if ($external -and -not (Test-Path -LiteralPath $projectPs2Dev)) {
        Write-Host "A valid PS2DEV already exists at: $external"
        if (Ask-YesNo "Create project\psdevwindows as a junction to the existing PS2DEV?") {
            try {
                New-Item -ItemType Junction -Path $projectPs2Dev -Target $external | Out-Null
            } catch {
                Write-Warning "Could not create the junction: $($_.Exception.Message)"
            }
        }

        if (Test-Ps2Dev $projectPs2Dev) {
            return $projectPs2Dev
        }
    }

    Write-Host "PS2DEV was not found at the location required by the OptiJuegos PS2 preset:"
    Write-Host "  $projectPs2Dev"

    if (-not (Ask-YesNo "Download the official precompiled PS2DEV toolchain into psdevwindows now?")) {
        Fail "PS2DEV is required for PS2 builds."
    }

    Install-Ps2Dev $projectPs2Dev

    if (-not (Test-Ps2Dev $projectPs2Dev)) {
        Fail "PS2DEV installation is incomplete: $projectPs2Dev"
    }

    return $projectPs2Dev
}

function Set-Ps2Environment([string]$Ps2Dev) {
    $env:PS2DEV = $Ps2Dev
    $env:PS2SDK = Join-Path $Ps2Dev "ps2sdk"
    $env:GSKIT = Join-Path $Ps2Dev "gsKit"

    $env:Path = @(
        (Join-Path $Ps2Dev "bin"),
        (Join-Path $Ps2Dev "ee\bin"),
        (Join-Path $Ps2Dev "iop\bin"),
        (Join-Path $Ps2Dev "dvp\bin"),
        (Join-Path $env:PS2SDK "bin"),
        $env:Path
    ) -join ";"
}

function Find-Gcc {
    foreach ($name in @("gcc.exe", "gcc")) {
        $cmd = Get-Command $name -ErrorAction SilentlyContinue
        if ($cmd) { return $cmd.Source }
    }

    $wingetPackages = Join-Path $env:LOCALAPPDATA "Microsoft\WinGet\Packages"
    if (Test-Path -LiteralPath $wingetPackages) {
        $candidate = Get-ChildItem -LiteralPath $wingetPackages -Filter "gcc.exe" -File -Recurse -ErrorAction SilentlyContinue |
            Where-Object { $_.FullName -match '\\bin\\gcc\.exe$' } |
            Select-Object -First 1

        if ($candidate) {
            $env:Path = "$($candidate.DirectoryName);$env:Path"
            return $candidate.FullName
        }
    }

    return $null
}

function Ensure-Gcc {
    $gcc = Find-Gcc
    if ($gcc) { return $gcc }

    if (-not (Ask-YesNo "MinGW-w64 GCC is missing. Install WinLibs GCC/MinGW-w64 now?")) {
        Fail "GCC/MinGW-w64 is required for the selected build."
    }

    Install-WingetPackage "BrechtSanders.WinLibs.MCF.UCRT"
    Refresh-Path
    $gcc = Find-Gcc

    if (-not $gcc) {
        Fail "GCC was not found after installing WinLibs."
    }

    return $gcc
}

function Test-Gcc32([string]$Gcc) {
    if (-not $Gcc) { return $false }

    $tempDir = Join-Path $env:TEMP ("mcps2-gcc32-" + [guid]::NewGuid().ToString("N"))
    New-Item -ItemType Directory -Force -Path $tempDir | Out-Null
    $src = Join-Path $tempDir "test.c"
    $exe = Join-Path $tempDir "test.exe"

    try {
        Set-Content -LiteralPath $src -Value "int main(void){return 0;}" -Encoding Ascii
        & $Gcc -m32 $src -o $exe *> $null
        return ($LASTEXITCODE -eq 0 -and (Test-Path -LiteralPath $exe))
    } catch {
        return $false
    } finally {
        Remove-Item -Recurse -Force -LiteralPath $tempDir -ErrorAction SilentlyContinue
    }
}

function Find-ProjectWinLibs32Bin {
    $base = Join-Path $Root "toolchains\winlibs32"
    if (-not (Test-Path -LiteralPath $base)) { return $null }

    $gcc = Get-ChildItem -LiteralPath $base -Filter "gcc.exe" -File -Recurse -ErrorAction SilentlyContinue |
        Where-Object { $_.FullName -match '\\bin\\gcc\.exe$' } |
        Select-Object -First 1

    if ($gcc) { return $gcc.DirectoryName }
    return $null
}

function Install-ProjectWinLibs32 {
    $base = Join-Path $Root "toolchains\winlibs32"
    if (Test-Path -LiteralPath $base) {
        Remove-Item -Recurse -Force -LiteralPath $base
    }
    New-Item -ItemType Directory -Force -Path $base | Out-Null

    Write-Host "Finding the latest WinLibs i686 GCC archive..."
    $headers = @{ "User-Agent" = "MCPS2-Windows-Build-Script" }
    $release = Invoke-RestMethod -Headers $headers -Uri "https://api.github.com/repos/brechtsanders/winlibs_mingw/releases/latest"

    $assets = @($release.assets)
    $asset = $assets |
        Where-Object {
            $_.name -match 'i686' -and
            $_.name -match 'ucrt' -and
            $_.name -match '\.zip$'
        } |
        Select-Object -First 1

    if (-not $asset) {
        # Fallback: any recent i686 ZIP from that release.
        $asset = $assets |
            Where-Object { $_.name -match 'i686' -and $_.name -match '\.zip$' } |
            Select-Object -First 1
    }

    if (-not $asset) {
        Fail "Could not find an i686 WinLibs ZIP in the latest release."
    }

    $archive = Join-Path $env:TEMP $asset.name
    Write-Host "Downloading $($asset.name) ..."
    Invoke-WebRequest -Headers $headers -Uri $asset.browser_download_url -OutFile $archive

    Write-Host "Extracting the 32-bit GCC toolchain into:"
    Write-Host "  $base"
    Expand-Archive -LiteralPath $archive -DestinationPath $base -Force
    Remove-Item -Force -LiteralPath $archive -ErrorAction SilentlyContinue
}

function Ensure-Gcc32 {
    $projectBin = Find-ProjectWinLibs32Bin
    if ($projectBin) {
        $env:Path = "$projectBin;$env:Path"
        $gcc = Find-Gcc
        if (Test-Gcc32 $gcc) { return $gcc }
    }

    $gcc = Ensure-Gcc
    if (Test-Gcc32 $gcc) { return $gcc }

    Write-Host "The current GCC does not successfully compile with -m32."
    if (-not (Ask-YesNo "Download a project-local 32-bit WinLibs GCC toolchain now?")) {
        Fail "A working 32-bit GCC toolchain is required."
    }

    Install-ProjectWinLibs32
    $projectBin = Find-ProjectWinLibs32Bin
    if (-not $projectBin) {
        Fail "The downloaded WinLibs archive did not contain gcc.exe."
    }

    $env:Path = "$projectBin;$env:Path"
    $gcc = Find-Gcc

    if (-not (Test-Gcc32 $gcc)) {
        Fail "The downloaded 32-bit GCC still failed the -m32 compiler test."
    }

    return $gcc
}

function Find-VSWhere {
    $candidate = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path -LiteralPath $candidate) { return $candidate }

    $cmd = Get-Command vswhere.exe -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }

    return $null
}

function Test-MSVCBuildTools {
    $vswhere = Find-VSWhere
    if (-not $vswhere) { return $false }

    try {
        $path = & $vswhere -latest -products * `
            -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
            -property installationPath

        return (-not [string]::IsNullOrWhiteSpace(($path | Select-Object -First 1)))
    } catch {
        return $false
    }
}

function Ensure-MSVCBuildTools {
    if (Test-MSVCBuildTools) { return }

    if (-not (Ask-YesNo "Visual Studio C++ Build Tools are missing. Install the Desktop C++ build workload now?")) {
        Fail "MSVC C++ Build Tools are required for the selected build."
    }

    $winget = Require-Winget
    & $winget install `
        --id "Microsoft.VisualStudio.2022.BuildTools" `
        -e `
        --accept-package-agreements `
        --accept-source-agreements `
        --override "--wait --passive --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"

    if ($LASTEXITCODE -ne 0) {
        Fail "winget failed to install Visual Studio Build Tools."
    }

    Refresh-Path

    if (-not (Test-MSVCBuildTools)) {
        Fail "Visual Studio C++ Build Tools were not detected after installation."
    }
}

function Test-DevkitProWii {
    $roots = @()
    if ($env:DEVKITPRO -and $env:DEVKITPRO -notmatch '^/') {
        $roots += $env:DEVKITPRO
    }
    $roots += "C:\devkitPro"

    foreach ($base in ($roots | Select-Object -Unique)) {
        if (
            (Test-Path -LiteralPath (Join-Path $base "devkitPPC\bin\powerpc-eabi-g++.exe")) -and
            (Test-Path -LiteralPath (Join-Path $base "libogc"))
        ) {
            $env:DEVKITPRO = $base
            $env:DEVKITPPC = Join-Path $base "devkitPPC"
            return $true
        }
    }

    return $false
}

function Find-DevkitProBash {
    foreach ($candidate in @(
        "C:\devkitPro\msys2\usr\bin\bash.exe",
        "C:\devkitPro\msys2\usr\bin\bash.exe",
        (Join-Path ${env:ProgramFiles(x86)} "devkitPro\msys2\usr\bin\bash.exe")
    )) {
        if ($candidate -and (Test-Path -LiteralPath $candidate)) {
            return $candidate
        }
    }

    return $null
}

function Install-DevkitProInstaller {
    $headers = @{ "User-Agent" = "MCPS2-Windows-Build-Script" }
    Write-Host "Finding the official devkitPro Windows installer..."
    $release = Invoke-RestMethod -Headers $headers -Uri "https://api.github.com/repos/devkitPro/installer/releases/latest"
    $asset = $release.assets |
        Where-Object { $_.name -match '\.exe$' } |
        Select-Object -First 1

    if (-not $asset) {
        Fail "The latest devkitPro installer release does not contain a Windows .exe asset."
    }

    $installer = Join-Path $env:TEMP $asset.name
    Write-Host "Downloading $($asset.name) ..."
    Invoke-WebRequest -Headers $headers -Uri $asset.browser_download_url -OutFile $installer

    Write-Host ""
    Write-Host "The official devkitPro installer will open."
    Write-Host "After it finishes, this script will install/check the Wii package group."
    Write-Host ""

    $process = Start-Process -FilePath $installer -Wait -PassThru
    Remove-Item -Force -LiteralPath $installer -ErrorAction SilentlyContinue

    if ($process.ExitCode -ne 0) {
        Fail "The devkitPro installer exited with code $($process.ExitCode)."
    }
}

function Ensure-DevkitProWii {
    if (Test-DevkitProWii) { return }

    $bash = Find-DevkitProBash
    if (-not $bash) {
        if (-not (Ask-YesNo "devkitPro/devkitPPC is missing. Download and run the official devkitPro installer now?")) {
            Fail "devkitPPC + libogc are required for Wii builds."
        }

        Install-DevkitProInstaller
        $bash = Find-DevkitProBash
    }

    if ($bash -and -not (Test-DevkitProWii)) {
        Write-Host "Installing/updating the devkitPro Wii package group (wii-dev) ..."
        & $bash -lc "pacman -S --needed --noconfirm wii-dev"
        if ($LASTEXITCODE -ne 0) {
            Fail "devkitPro pacman failed to install wii-dev."
        }
    }

    if (-not (Test-DevkitProWii)) {
        Fail "devkitPPC/libogc were not detected after installation."
    }

    $env:Path = @(
        (Join-Path $env:DEVKITPPC "bin"),
        (Join-Path $env:DEVKITPRO "tools\bin"),
        $env:Path
    ) -join ";"
}

function Find-7Zip {
    $cmd = Get-Command 7z.exe -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }

    foreach ($candidate in @(
        (Join-Path $env:ProgramFiles "7-Zip\7z.exe"),
        (Join-Path ${env:ProgramFiles(x86)} "7-Zip\7z.exe")
    )) {
        if ($candidate -and (Test-Path -LiteralPath $candidate)) {
            $env:Path = "$(Split-Path -Parent $candidate);$env:Path"
            return $candidate
        }
    }

    return $null
}

function Ensure-7Zip {
    $sevenZip = Find-7Zip
    if ($sevenZip) { return $sevenZip }

    if (-not (Ask-YesNo "7-Zip is missing. Install 7-Zip now?")) {
        Fail "7-Zip is required by create zip.bat."
    }

    Install-WingetPackage "7zip.7zip"
    $sevenZip = Find-7Zip

    if (-not $sevenZip) {
        Fail "7-Zip was not found after installation."
    }

    return $sevenZip
}

function Ensure-LegacyScriptDependencies([string]$ScriptName) {
    $name = $ScriptName.ToLowerInvariant()

    Write-Host ""
    Write-Host "Checking dependencies for: $ScriptName"

    if ($name -like "build ps2*.bat") {
        [void](Ensure-NativeCMake)
        [void](Ensure-RootNinja)
        $ps2dev = Ensure-ProjectPs2Dev
        Set-Ps2Environment $ps2dev
        return
    }

    if ($name -like "build wii*.bat") {
        [void](Ensure-NativeCMake)
        [void](Ensure-RootNinja)
        Ensure-DevkitProWii
        return
    }

    if ($name -like "build gcc*32*.bat") {
        [void](Ensure-NativeCMake)
        Ensure-GccPresetNinja
        [void](Ensure-Gcc32)
        return
    }

    if ($name -like "build gcc*.bat") {
        [void](Ensure-NativeCMake)
        Ensure-GccPresetNinja
        [void](Ensure-Gcc)
        return
    }

    if ($name -like "build msvc*.bat") {
        [void](Ensure-NativeCMake)
        Ensure-MSVCBuildTools
        return
    }

    if ($name -eq "create zip.bat") {
        [void](Ensure-7Zip)
        return
    }

    if ($name -eq "update time.bat") {
        # The original script is a Windows batch/PowerShell utility. No
        # additional compiler/toolchain dependency is required here.
        return
    }

    Write-Warning "No dependency profile is defined for '$ScriptName'. It will be executed as-is."
}

function Get-LegacyScripts {
    if (-not (Test-Path -LiteralPath $LegacyDir -PathType Container)) {
        return @()
    }

    # !!del GUARDA!!.bat is intentionally kept out of the menu. It belongs in
    # the project root and is destructive.
    return @(
        Get-ChildItem -LiteralPath $LegacyDir -Filter "*.bat" -File |
            Where-Object { $_.Name -ne "!!del GUARDA!!.bat" } |
            Sort-Object Name
    )
}

function Show-BuildSelectionMenu {
    $legacyScripts = @(Get-LegacyScripts)

    Write-Host ""
    Write-Host "============================================================"
    Write-Host "OptiCraft / MC:PS2 - Windows build selection"
    Write-Host "============================================================"
    Write-Host "[0] Standard automatic PS2 build (network + sound)"

    for ($i = 0; $i -lt $legacyScripts.Count; $i++) {
        Write-Host ("[{0}] OptiJuegos: {1}" -f ($i + 1), $legacyScripts[$i].Name)
    }

    Write-Host "[Q] Quit"
    Write-Host ""

    while ($true) {
        $choice = Read-Host "Select a build [0]"

        if ([string]::IsNullOrWhiteSpace($choice) -or $choice.Trim() -eq "0") {
            return $null
        }

        if ($choice.Trim().ToLowerInvariant() -eq "q") {
            exit 0
        }

        $number = 0
        if ([int]::TryParse($choice.Trim(), [ref]$number)) {
            if ($number -ge 1 -and $number -le $legacyScripts.Count) {
                return $legacyScripts[$number - 1]
            }
        }

        Write-Warning "Invalid selection."
    }
}

function Invoke-OptiJuegosBuildScript([System.IO.FileInfo]$SelectedScript) {
    Ensure-LegacyScriptDependencies $SelectedScript.Name

    $legacyScripts = @(Get-LegacyScripts)
    if ($legacyScripts.Count -eq 0) {
        Fail "No .bat files were found in $LegacyDir"
    }

    # The original batch files were written to live in the project root and
    # commonly use %~dp0. Copy every non-destructive sibling .bat temporarily
    # so relative calls between the legacy scripts continue to work.
    $temporaryCopies = @()

    try {
        foreach ($script in $legacyScripts) {
            $destination = Join-Path $Root $script.Name
            if (Test-Path -LiteralPath $destination) {
                Fail "Cannot run '$($script.Name)': a file with the same name already exists in the project root: $destination"
            }

            Copy-Item -LiteralPath $script.FullName -Destination $destination
            $temporaryCopies += $destination
        }

        $rootScript = Join-Path $Root $SelectedScript.Name

        Write-Host ""
        Write-Host "============================================================"
        Write-Host "Running OptiJuegos build script"
        Write-Host "============================================================"
        Write-Host "Script: $($SelectedScript.Name)"
        Write-Host "Root:   $Root"
        Write-Host ""

        Push-Location $Root
        try {
            & $env:ComSpec /d /c "call `"$rootScript`""
            $exitCode = $LASTEXITCODE
        }
        finally {
            Pop-Location
        }

        if ($exitCode -ne 0) {
            Fail "The selected OptiJuegos build script exited with code $exitCode."
        }

        Write-Host ""
        Write-Host "OptiJuegos build script completed successfully."
    }
    finally {
        foreach ($copy in $temporaryCopies) {
            Remove-Item -LiteralPath $copy -Force -ErrorAction SilentlyContinue
        }
    }
}

function Assert-StandardPs2Layout {
    foreach ($required in @(
        "CMakeLists.txt",
        "cmake\ps2_toolchain.cmake",
        "scripts\make_pak.py"
    )) {
        $path = Join-Path $Root $required
        if (-not (Test-Path -LiteralPath $path)) {
            Fail "Required file/path not found for the standard PS2 build: $required"
        }
    }

    foreach ($candidate in @(
        "data\assets",
        "assets"
    )) {
        if (Test-Path -LiteralPath (Join-Path $Root $candidate)) {
            return
        }
    }

    Write-Warning "Could not find data\assets or assets. The current project tree may use a different asset layout; CMake will report the exact problem if staging needs it."
}

function Invoke-StandardPs2Build {
    Assert-StandardPs2Layout

    $CMake = Ensure-NativeCMake
    $Ninja = Ensure-RootNinja
    $Python = Ensure-Python
    $Ps2Dev = Ensure-ProjectPs2Dev
    Set-Ps2Environment $Ps2Dev

    if ($Clean -and (Test-Path -LiteralPath $BuildDir)) {
        Write-Host "Removing $BuildDir ..."
        Remove-Item -Recurse -Force -LiteralPath $BuildDir
    }

    New-Item -ItemType Directory -Force -Path $AppDir | Out-Null
    $Jobs = [Environment]::ProcessorCount
    if ($Jobs -lt 1) { $Jobs = 4 }

    Write-Host ""
    Write-Host "============================================================"
    Write-Host "MC:PS2 - standard PS2 build"
    Write-Host "============================================================"
    Write-Host "Project: $Root"
    Write-Host "PS2DEV:  $Ps2Dev"
    Write-Host "CMake:   $CMake"
    Write-Host "Ninja:   $Ninja"
    Write-Host "Build:   $BuildDir"
    Write-Host "Jobs:    $Jobs"
    Write-Host "Network: ON"
    Write-Host "Sound:   ON"
    Write-Host ""

    $configureArgs = @(
        "-S", $Root,
        "-B", $BuildDir,
        "-G", "Ninja",
        "-DCMAKE_MAKE_PROGRAM=$Ninja",
        "-DCMAKE_TOOLCHAIN_FILE=$(Join-Path $Root 'cmake\ps2_toolchain.cmake')",
        "-DPLATFORM=PS2",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DPS2_ENABLE_NETWORK=ON",
        "-DPS2_ENABLE_SOUND=ON",
        "-DMC_LOG_LEVEL=0",
        "-DPS2_RENDER_STATS=OFF",
        "-DPS2_ENABLE_PERSPECTIVE_TEXTURES=ON",
        "-DPS2_ENABLE_VU1_TERRAIN=ON"
    )

    & $CMake @configureArgs
    if ($LASTEXITCODE -ne 0) { Fail "CMake configure failed." }

    & $CMake --build $BuildDir --target OptiCraft --parallel $Jobs
    if ($LASTEXITCODE -ne 0) { Fail "OptiCraft build failed." }

    & $CMake --build $BuildDir --target ps2-data --parallel $Jobs
    if ($LASTEXITCODE -ne 0) { Fail "ps2-data staging failed." }

    $pythonArgs = @($Python.PrefixArgs) + @(
        (Join-Path $Root "scripts\make_pak.py"),
        (Join-Path $AppDir "data"),
        $Pak
    )

    & $Python.Exe @pythonArgs
    if ($LASTEXITCODE -ne 0) { Fail "assets.pak generation failed." }

    if (-not (Test-Path -LiteralPath $Elf) -or (Get-Item -LiteralPath $Elf).Length -eq 0) {
        Fail "OptiCraft.elf was not created: $Elf"
    }

    if (-not (Test-Path -LiteralPath $Pak) -or (Get-Item -LiteralPath $Pak).Length -eq 0) {
        Fail "assets.pak was not created: $Pak"
    }

    foreach ($irx in @("audsrv", "ps2dev9", "netman", "smap")) {
        $irxPath = Join-Path $AppDir "data\irx\$irx.irx"
        if (-not (Test-Path -LiteralPath $irxPath) -or (Get-Item -LiteralPath $irxPath).Length -eq 0) {
            Fail "Missing staged IRX: $irxPath"
        }
    }

    $bytes = [IO.File]::ReadAllBytes($Pak)
    if ($bytes.Length -lt 32 -or [Text.Encoding]::ASCII.GetString($bytes, 0, 4) -ne "MCPK") {
        Fail "assets.pak does not have a valid MCPK header."
    }

    if (Test-Path -LiteralPath $Map) {
        $mapText = Get-Content -LiteralPath $Map -Raw
        $match = [regex]::Match($mapText, '(?m)^\s*0x([0-9a-fA-F]+)\s+errno\s*$')

        if ($match.Success) {
            $errnoAddress = [Convert]::ToInt64($match.Groups[1].Value, 16)
            Write-Host ("errno: 0x{0:x8} (alignment mod 4 = {1})" -f $errnoAddress, ($errnoAddress % 4))

            if (($errnoAddress % 4) -ne 0) {
                Fail "errno is not 4-byte aligned; refusing to publish this build."
            }
        }
    }

    Write-Host ""
    Write-Host "SHA-256:"
    Get-FileHash -Algorithm SHA256 -Path @($Elf, $Pak) | Format-Table -AutoSize

    Write-Host "============================================================"
    Write-Host "BUILD COMPLETE"
    Write-Host "============================================================"
    Write-Host "ELF: $Elf"
    Write-Host "PAK: $Pak"
    Write-Host ""
}

if (-not (Test-Path -LiteralPath (Join-Path $Root "CMakeLists.txt"))) {
    Fail "Place build_ps2_windows.ps1 in the project root (CMakeLists.txt not found)."
}

$legacySelection = Show-BuildSelectionMenu
if ($null -ne $legacySelection) {
    Invoke-OptiJuegosBuildScript $legacySelection
    exit 0
}

Invoke-StandardPs2Build
