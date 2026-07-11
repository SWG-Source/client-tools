[CmdletBinding()]
param(
    [ValidateSet("Release", "Optimized", "Debug")]
    [string]$Configuration = "Release",

    [string]$PlatformToolset = "v145",

    [string]$VisualStudioRoot,

    [string]$StagePath
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Get-MsBuildPath {
    param([string]$RequestedVisualStudioRoot)

    if ($RequestedVisualStudioRoot) {
        $candidate = Join-Path $RequestedVisualStudioRoot "MSBuild\Current\Bin\MSBuild.exe"
        if (-not (Test-Path -LiteralPath $candidate -PathType Leaf)) {
            throw "MSBuild was not found below VisualStudioRoot: $RequestedVisualStudioRoot"
        }

        return (Resolve-Path -LiteralPath $candidate).Path
    }

    $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path -LiteralPath $vswhere -PathType Leaf) {
        $matches = @(
            & $vswhere -latest -products * -requires Microsoft.Component.MSBuild `
                -find "MSBuild\**\Bin\MSBuild.exe"
        )
        if ($matches.Count -gt 0) {
            return $matches[0]
        }
    }

    $command = Get-Command msbuild.exe -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    throw "MSBuild was not found. Install Visual Studio Build Tools with Desktop development with C++."
}

function Get-PeMachine {
    param([Parameter(Mandatory)][string]$Path)

    $stream = [IO.File]::OpenRead($Path)
    $reader = [IO.BinaryReader]::new($stream)
    try {
        if ($reader.ReadUInt16() -ne 0x5a4d) {
            throw "Not a PE file: $Path"
        }

        $stream.Position = 0x3c
        $peOffset = $reader.ReadInt32()
        $stream.Position = $peOffset
        if ($reader.ReadUInt32() -ne 0x00004550) {
            throw "Invalid PE signature: $Path"
        }

        return $reader.ReadUInt16()
    }
    finally {
        $reader.Dispose()
        $stream.Dispose()
    }
}

$repoRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..")).Path
$solution = Join-Path $repoRoot "src\build\win32\swg.sln"
$msbuild = Get-MsBuildPath -RequestedVisualStudioRoot $VisualStudioRoot

if (-not $env:DXSDK_DIR) {
    $defaultDirectX = Join-Path ${env:ProgramFiles(x86)} "Microsoft DirectX SDK (June 2010)"
    if (Test-Path -LiteralPath $defaultDirectX -PathType Container) {
        $env:DXSDK_DIR = $defaultDirectX.TrimEnd("\") + "\"
    }
}

if (-not $env:DXSDK_DIR -or -not (Test-Path -LiteralPath $env:DXSDK_DIR -PathType Container)) {
    throw "Set DXSDK_DIR to the Microsoft DirectX SDK (June 2010) installation directory."
}

$requiredInputs = @(
    "deps\x64\lib\libxml2.lib",
    "deps\x64\lib\pcre.lib",
    "deps\x64\lib\dpvs.lib",
    "deps\x64\lib\libEverQuestTCG.lib",
    "deps\x64\lib\vivoxSharedWrapper.lib",
    "mss64-stub\mss64.lib"
)

foreach ($relativePath in $requiredInputs) {
    $path = Join-Path $repoRoot $relativePath
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "Required vendored x64 input is missing: $path"
    }
}

$arguments = @(
    $solution,
    "/t:SwgClient",
    "/p:Configuration=$Configuration",
    "/p:Platform=x64",
    "/p:PlatformToolset=$PlatformToolset",
    "/m",
    "/nr:false",
    "/v:minimal"
)

Write-Host "MSBuild: $msbuild"
Write-Host "Solution: $solution"
Write-Host "Configuration: $Configuration|x64 ($PlatformToolset)"

& $msbuild @arguments
if ($LASTEXITCODE -ne 0) {
    throw "The x64 client build failed with exit code $LASTEXITCODE."
}

$suffix = @{
    Release   = "r"
    Optimized = "o"
    Debug     = "d"
}[$Configuration]

$artifacts = @(
    "src\build\win32\x64\$Configuration\SwgClient_$suffix.exe",
    "src\build\win32\x64\$Configuration\gl05_$suffix.dll",
    "src\build\win32\x64\$Configuration\gl06_$suffix.dll",
    "src\build\win32\x64\$Configuration\gl07_$suffix.dll",
    "src\build\win32\x64\$Configuration\DllExport.dll"
)

foreach ($relativePath in $artifacts) {
    $path = Join-Path $repoRoot $relativePath
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "Expected build artifact is missing: $path"
    }

    $machine = Get-PeMachine -Path $path
    if ($machine -ne 0x8664) {
        throw ("Expected x64 PE machine 0x8664, found 0x{0:x4}: {1}" -f $machine, $path)
    }

    $item = Get-Item -LiteralPath $path
    $hash = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash
    Write-Host ("x64  {0,10:N0} bytes  {1}  {2}" -f $item.Length, $hash, $path)
}

if ($StagePath) {
    & (Join-Path $PSScriptRoot "Stage-X64Client.ps1") `
        -ClientRoot $StagePath `
        -Configuration $Configuration
}
