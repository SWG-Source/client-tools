[CmdletBinding()]
param(
    [ValidateSet("Release")]
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
$dpvsProject = Join-Path $repoRoot "src\external\3rd\library\dpvs\implementation\msvc8\dpvs.vcxproj"
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
    "deps\win32\bin\msvcr71.dll",
    "src\external\3rd\library\qt\3.3.4\lib\qt-mt334.lib",
    "src\external\3rd\library\qt\3.3.4\lib\qt-mt334.dll"
)

foreach ($relativePath in $requiredInputs) {
    $path = Join-Path $repoRoot $relativePath
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "Required Win32 God client input is missing: $path"
    }
}

Write-Host "MSBuild: $msbuild"
Write-Host "Configuration: $Configuration|Win32 ($PlatformToolset)"
Write-Host "Building DPVS from local source..."

$commonArguments = @(
    "/p:Configuration=$Configuration",
    "/p:Platform=Win32",
    "/p:PlatformToolset=$PlatformToolset",
    "/m",
    "/nr:false",
    "/v:minimal"
)

& $msbuild $dpvsProject @commonArguments
if ($LASTEXITCODE -ne 0) {
    throw "The Win32 DPVS build failed with exit code $LASTEXITCODE."
}

Write-Host "Building the God client and renderer modules..."
& $msbuild $solution "/t:SwgGodClient" @commonArguments
if ($LASTEXITCODE -ne 0) {
    throw "The Win32 God client build failed with exit code $LASTEXITCODE."
}

$artifacts = @(
    "src\compile\win32\dpvs\$Configuration\dpvs.dll",
    "src\compile\win32\SwgGodClient\$Configuration\SwgGodClient_r.exe",
    "src\compile\win32\Direct3d9\$Configuration\gl05_r.dll",
    "src\compile\win32\Direct3d9_ffp\$Configuration\gl06_r.dll",
    "src\compile\win32\Direct3d9_vsps\$Configuration\gl07_r.dll",
    "src\compile\win32\DllExport\$Configuration\DllExport.dll"
)

foreach ($relativePath in $artifacts) {
    $path = Join-Path $repoRoot $relativePath
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "Expected build artifact is missing: $path"
    }

    $machine = Get-PeMachine -Path $path
    if ($machine -ne 0x014c) {
        throw ("Expected x86 PE machine 0x014c, found 0x{0:x4}: {1}" -f $machine, $path)
    }

    $item = Get-Item -LiteralPath $path
    $hash = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash
    Write-Host ("x86  {0,10:N0} bytes  {1}  {2}" -f $item.Length, $hash, $path)
}

if ($StagePath) {
    & (Join-Path $PSScriptRoot "Stage-GodClient.ps1") `
        -GodClientRoot $StagePath `
        -Configuration $Configuration
}
