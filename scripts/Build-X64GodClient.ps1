[CmdletBinding()]
param(
    [ValidateSet("Release")]
    [string]$Configuration = "Release",

    [string]$PlatformToolset = "v145",

    [string]$VisualStudioRoot,

    [string]$StagePath,

    [switch]$SkipQtBuild
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

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
$prerequisites = & (Join-Path $PSScriptRoot "Test-X64BuildPrerequisites.ps1") `
    -PlatformToolset $PlatformToolset `
    -VisualStudioRoot $VisualStudioRoot `
    -Quiet `
    -PassThru
$vsRoot = $prerequisites.VisualStudio.Root
$devShell = Join-Path $vsRoot "Common7\Tools\Launch-VsDevShell.ps1"
$msbuild = Join-Path $vsRoot "MSBuild\Current\Bin\MSBuild.exe"

if (-not (Test-Path -LiteralPath $devShell -PathType Leaf) -or -not (Test-Path -LiteralPath $msbuild -PathType Leaf)) {
    throw "Visual Studio build tools are incomplete below: $vsRoot"
}

if (-not $SkipQtBuild) {
    & (Join-Path $PSScriptRoot "Build-X64Qt3.ps1") -VisualStudioRoot $vsRoot
}

& $devShell -Arch amd64 -HostArch amd64 -SkipAutomaticLocation
$env:DXSDK_DIR = $prerequisites.DirectXSdk.Root.TrimEnd("\") + "\"

$requiredInputs = @(
    "deps\x64\include\libjpeg-turbo\jpeglib.h",
    "deps\x64\lib\jpeg-static.lib",
    "deps\x64\lib\dpvs.lib",
    "deps\x64\lib\libxml2.lib",
    "deps\x64\lib\pcre.lib",
    "deps\x64\lib\libEverQuestTCG.lib",
    "deps\x64\lib\vivoxSharedWrapper.lib",
    "deps\x64\lib\swg-stubs.lib",
    "deps\x64\compat-source\misc_stubs.cpp",
    "deps\qt3-win64-src\lib\qt-mt3.lib",
    "deps\qt3-win64-src\lib\qtmain.lib",
    "deps\qt3-win64-src\lib\qt-mt3.dll"
)
foreach ($relativePath in $requiredInputs) {
    $path = Join-Path $repoRoot $relativePath
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "Required local x64 God client input is missing: $path"
    }
}

$compatSourceRoot = Join-Path $repoRoot "deps\x64\compat-source"
$compatLibraryRoot = Join-Path $repoRoot "deps\x64\lib"
$compatObject = Join-Path $compatLibraryRoot "lgLcd_stubs.obj"
$compatLibrary = Join-Path $compatLibraryRoot "lgLcd.lib"
Push-Location $compatSourceRoot
try {
    & cl.exe /nologo /c /O2 /MT /EHsc misc_stubs.cpp "/Fo:$compatObject"
    if ($LASTEXITCODE -ne 0) {
        throw "The x64 LCD compatibility stub compile failed with exit code $LASTEXITCODE."
    }
    & lib.exe /nologo /MACHINE:X64 "/OUT:$compatLibrary" $compatObject
    if ($LASTEXITCODE -ne 0) {
        throw "The x64 LCD compatibility library build failed with exit code $LASTEXITCODE."
    }
}
finally {
    Pop-Location
}

$arguments = @(
    $solution,
    "/t:SwgGodClient",
    "/p:Configuration=$Configuration",
    "/p:Platform=x64",
    "/p:PlatformToolset=$PlatformToolset",
    "/m",
    "/nr:false",
    "/v:minimal"
)

Write-Host "MSBuild: $msbuild"
Write-Host "Configuration: $Configuration|x64 ($PlatformToolset)"
& $msbuild @arguments
if ($LASTEXITCODE -ne 0) {
    throw "The x64 God client build failed with exit code $LASTEXITCODE."
}

$artifacts = @(
    "src\build\win32\x64\$Configuration\SwgGodClient_r.exe",
    "src\build\win32\x64\$Configuration\gl05_r.dll",
    "src\build\win32\x64\$Configuration\gl06_r.dll",
    "src\build\win32\x64\$Configuration\gl07_r.dll",
    "src\build\win32\x64\$Configuration\DllExport.dll",
    "deps\qt3-win64-src\lib\qt-mt3.dll",
    "deps\x64\bin\libxml2.dll",
    "deps\x64\bin\iconv-2.dll",
    "deps\x64\bin\z.dll"
)
foreach ($relativePath in $artifacts) {
    $path = Join-Path $repoRoot $relativePath
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "Expected x64 God client artifact is missing: $path"
    }
    if ((Get-PeMachine -Path $path) -ne 0x8664) {
        throw "Expected an x64 God client artifact: $path"
    }
    $item = Get-Item -LiteralPath $path
    $hash = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash
    Write-Host ("x64  {0,10:N0} bytes  {1}  {2}" -f $item.Length, $hash, $path)
}

if ($StagePath) {
    & (Join-Path $PSScriptRoot "Stage-X64GodClient.ps1") `
        -GodClientRoot $StagePath `
        -Configuration $Configuration
}
