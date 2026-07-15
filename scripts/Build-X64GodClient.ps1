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
    "deps\x64\include\SDL3\SDL.h",
    "deps\x64\lib\SDL3.lib",
    "deps\x64\bin\SDL3.dll",
    "deps\x64\include\libjpeg-turbo\jpeglib.h",
    "deps\x64\lib\jpeg-static.lib",
    "deps\x64\lib\dpvs.lib",
    "deps\x64\lib\libxml2.lib",
    "deps\x64\lib\pcre.lib",
    "deps\x64\lib\libEverQuestTCG.lib",
    "deps\x64\lib\vivoxSharedWrapper.lib",
    "deps\qt3-win64-src\lib\qt-mt3.lib",
    "deps\qt3-win64-src\lib\qtmain.lib",
    "deps\qt3-win64-src\lib\qt-mt3.dll",
    "src\external\3rd\library\qt\3.3.4\bin\uic.exe",
    "src\external\3rd\library\qt\3.3.4\bin\moc.exe",
    "mss64-stub\mss64.lib"
)
foreach ($relativePath in $requiredInputs) {
    $path = Join-Path $repoRoot $relativePath
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "Required local x64 God client input is missing: $path"
    }
}

# The converted project can compile generated UI sources before its CustomBuild
# items run, so generate them explicitly to make clean worktrees deterministic.
$uiRoot = Join-Path $repoRoot "src\game\client\application\SwgGodClient\src\shared\ui"
$uiOutputRoot = Join-Path $repoRoot "src\compile\win32\SwgGodClient\$Configuration"
$uic = Join-Path $repoRoot "src\external\3rd\library\qt\3.3.4\bin\uic.exe"
$moc = Join-Path $repoRoot "src\external\3rd\library\qt\3.3.4\bin\moc.exe"
[void](New-Item -ItemType Directory -Path $uiOutputRoot -Force)

foreach ($uiFile in Get-ChildItem -LiteralPath $uiRoot -Filter "*.ui" -File | Sort-Object Name) {
    $header = Join-Path $uiOutputRoot ($uiFile.BaseName + ".h")
    $source = Join-Path $uiOutputRoot ($uiFile.BaseName + "_r.cpp")

    & $uic -o $header $uiFile.FullName
    if ($LASTEXITCODE -ne 0) {
        throw "uic failed to generate $header."
    }
    & $uic -o $source -impl $header $uiFile.FullName
    if ($LASTEXITCODE -ne 0) {
        throw "uic failed to generate $source."
    }
    & $moc $header | Out-File -LiteralPath $source -Append -Encoding ascii
    if ($LASTEXITCODE -ne 0) {
        throw "moc failed to generate metadata for $header."
    }
}

$stubRoot = Join-Path $repoRoot "mss64-stub"
Push-Location $stubRoot
try {
    & cl.exe /nologo /c /O2 /MT /EHsc misc_stubs.cpp /Fo:lgLcd_stubs.obj
    if ($LASTEXITCODE -ne 0) {
        throw "The x64 LCD compatibility stub compile failed with exit code $LASTEXITCODE."
    }
    & lib.exe /nologo /MACHINE:X64 /OUT:lgLcd.lib lgLcd_stubs.obj
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
    "deps\x64\bin\SDL3.dll",
    "mss64-stub\mss64.dll",
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
