param(
    [string]$InputPath = "data\assets",
    [string]$OutputPath = "build\ps2-audio",
    [ValidateSet("Both", "Wav", "Adp")]
    [string]$Mode = "Both",
    [int]$SampleRate = 22050,
    [ValidateSet("mono", "stereo")]
    [string]$Channels = "mono",
    [string]$Ffmpeg = "ffmpeg",
    # IMPORTANT: audsrv_load_adpcm() expects the single-sample SPU-ADPCM format
    # produced by adpenc ("sadenc"), the same tool the official ps2sdk playadpcm
    # sample uses (`adpenc <in.wav> <out.adp>`). ps2adpcm.exe produces a
    # chunked/interleaved streaming format that audsrv cannot play -> silence.
    [string]$Ps2Adpcm = "psdevwindows\ps2sdk\bin\adpenc.exe",
    [switch]$Loop,
    [switch]$KeepWav,
    [switch]$SkipFailed,
    [switch]$Overwrite
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot

function Resolve-RepoPath {
    param([string]$Path)

    if ([System.IO.Path]::IsPathRooted($Path)) {
        return [System.IO.Path]::GetFullPath($Path)
    }

    return [System.IO.Path]::GetFullPath((Join-Path $repoRoot $Path))
}

function Require-Tool {
    param(
        [string]$Tool,
        [string]$Name
    )

    if ([System.IO.Path]::IsPathRooted($Tool) -or $Tool.Contains("\") -or $Tool.Contains("/")) {
        $toolPath = Resolve-RepoPath $Tool
        if (-not (Test-Path -LiteralPath $toolPath)) {
            throw "$Name not found: $toolPath"
        }
        return $toolPath
    }

    $cmd = Get-Command $Tool -ErrorAction SilentlyContinue
    if ($null -eq $cmd) {
        throw "$Name not found in PATH: $Tool"
    }

    return $cmd.Source
}

function Get-RelativePath {
    param(
        [string]$BasePath,
        [string]$FullPath
    )

    $baseUri = [System.Uri](([System.IO.Path]::GetFullPath($BasePath).TrimEnd("\", "/") + [System.IO.Path]::DirectorySeparatorChar))
    $fullUri = [System.Uri]([System.IO.Path]::GetFullPath($FullPath))
    return [System.Uri]::UnescapeDataString($baseUri.MakeRelativeUri($fullUri).ToString()).Replace("/", "\")
}

function Remove-RelativeExtension {
    param([string]$RelativePath)

    $dir = Split-Path -Parent $RelativePath
    $name = [System.IO.Path]::GetFileNameWithoutExtension($RelativePath)

    if ([string]::IsNullOrEmpty($dir)) {
        return $name
    }

    return Join-Path $dir $name
}

function Convert-OggToWav {
    param(
        [string]$Source,
        [string]$Target,
        [string]$FfmpegPath,
        [int]$Rate,
        [string]$ChannelMode
    )

    $targetDir = Split-Path -Parent $Target
    New-Item -ItemType Directory -Force -Path $targetDir | Out-Null

    if ((Test-Path -LiteralPath $Target) -and -not $Overwrite) {
        Write-Host "skip wav: $Target"
        return
    }

    $channelCount = if ($ChannelMode -eq "stereo") { "2" } else { "1" }
    $args = @(
        "-y",
        "-hide_banner",
        "-loglevel", "error",
        "-i", $Source,
        "-vn",
        "-ac", $channelCount,
        "-ar", "$Rate",
        "-sample_fmt", "s16",
        $Target
    )

    Write-Host "ogg -> wav: $Source"
    & $FfmpegPath @args
    if ($LASTEXITCODE -ne 0) {
        throw "ffmpeg failed for: $Source"
    }
}

function Convert-WavToAdp {
    param(
        [string]$Source,
        [string]$Target,
        [string]$AdpcmPath,
        [string]$ChannelMode
    )

    $targetDir = Split-Path -Parent $Target
    New-Item -ItemType Directory -Force -Path $targetDir | Out-Null

    if ((Test-Path -LiteralPath $Target) -and -not $Overwrite) {
        Write-Host "skip adp: $Target"
        return
    }

    # adpenc syntax: adpenc [-L] <input wave> <output adp>
    # Stereo is auto-detected from the WAV; there is no -s flag. -L marks the
    # sample as looping (only meaningful for sustained loops/ambient).
    $args = @()
    if ($Loop) {
        $args += "-L"
    }
    $args += @($Source, $Target)

    Write-Host "wav -> adp: $Source"
    & $AdpcmPath @args
    if ($LASTEXITCODE -ne 0) {
        throw "adpenc failed for: $Source"
    }
}

$inputRoot = Resolve-RepoPath $InputPath
$outputRoot = Resolve-RepoPath $OutputPath
$ffmpegPath = Require-Tool $Ffmpeg "ffmpeg"
$adpcmPath = Require-Tool $Ps2Adpcm "adpenc"

if (-not (Test-Path -LiteralPath $inputRoot)) {
    throw "Input path not found: $inputRoot"
}

New-Item -ItemType Directory -Force -Path $outputRoot | Out-Null

$converted = 0
$failed = 0

if ($Mode -eq "Both" -or $Mode -eq "Wav") {
    $oggFiles = Get-ChildItem -LiteralPath $inputRoot -Recurse -File -Filter *.ogg

    foreach ($ogg in $oggFiles) {
        try {
            if ($ogg.Length -eq 0) {
                throw "input file is empty"
            }

            $relative = Get-RelativePath $inputRoot $ogg.FullName
            $relativeNoExt = Remove-RelativeExtension $relative
            $wavPath = Join-Path $outputRoot ($relativeNoExt + ".wav")

            Convert-OggToWav $ogg.FullName $wavPath $ffmpegPath $SampleRate $Channels
            $converted++

            if ($Mode -eq "Both") {
                $adpPath = Join-Path $outputRoot ($relativeNoExt + ".adp")
                Convert-WavToAdp $wavPath $adpPath $adpcmPath $Channels

                if (-not $KeepWav) {
                    Remove-Item -LiteralPath $wavPath -Force
                }
            }
        }
        catch {
            if (-not $SkipFailed) {
                throw
            }

            $failed++
            Write-Warning "skip failed file: $($ogg.FullName) ($($_.Exception.Message))"
        }
    }
}
elseif ($Mode -eq "Adp") {
    $wavFiles = Get-ChildItem -LiteralPath $inputRoot -Recurse -File -Filter *.wav

    foreach ($wav in $wavFiles) {
        try {
            if ($wav.Length -eq 0) {
                throw "input file is empty"
            }

            $relative = Get-RelativePath $inputRoot $wav.FullName
            $relativeNoExt = Remove-RelativeExtension $relative
            $adpPath = Join-Path $outputRoot ($relativeNoExt + ".adp")

            Convert-WavToAdp $wav.FullName $adpPath $adpcmPath $Channels
            $converted++
        }
        catch {
            if (-not $SkipFailed) {
                throw
            }

            $failed++
            Write-Warning "skip failed file: $($wav.FullName) ($($_.Exception.Message))"
        }
    }
}

Write-Host "Done. Converted $converted file(s) into $outputRoot"
if ($failed -gt 0) {
    Write-Warning "Skipped $failed failed file(s)."
}
