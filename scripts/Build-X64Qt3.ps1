[CmdletBinding()]
param(
    [string]$VisualStudioRoot
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Get-VisualStudioRoot {
    param([string]$RequestedRoot)

    if ($RequestedRoot) {
        $root = (Resolve-Path -LiteralPath $RequestedRoot).Path
        if (-not (Test-Path -LiteralPath (Join-Path $root "Common7\Tools\Launch-VsDevShell.ps1") -PathType Leaf)) {
            throw "VisualStudioRoot is not a Visual Studio installation: $root"
        }
        return $root
    }

    $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path -LiteralPath $vswhere -PathType Leaf)) {
        throw "vswhere.exe was not found. Run .\scripts\Setup-X64BuildPrerequisites.ps1 -Install or pass -VisualStudioRoot explicitly."
    }

    $root = (& $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath).Trim()
    if (-not $root) {
        throw "Visual Studio with the x64 C++ toolchain was not found. Run .\scripts\Setup-X64BuildPrerequisites.ps1 -Install."
    }
    return $root
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
$qtRoot = Join-Path $repoRoot "deps\qt3-win64-src"
$vsRoot = Get-VisualStudioRoot -RequestedRoot $VisualStudioRoot
$devShell = Join-Path $vsRoot "Common7\Tools\Launch-VsDevShell.ps1"

if (-not (Test-Path -LiteralPath (Join-Path $qtRoot "qmake\Makefile") -PathType Leaf)) {
    throw "The prepared local Qt 3 x64 source tree is incomplete: $qtRoot"
}

& $devShell -Arch amd64 -HostArch amd64 -SkipAutomaticLocation
$env:QTDIR = $qtRoot
$env:QMAKESPEC = "win32-msvc2005"

# Qt 3's generated nmake files assume their output directories already exist.
$qtBuildDirectories = @(
    (Join-Path $qtRoot "bin"),
    (Join-Path $qtRoot "lib"),
    (Join-Path $qtRoot "qmake\tmp"),
    (Join-Path $qtRoot "src\moc\tmp\obj\release-shared-mt"),
    (Join-Path $qtRoot "src\moc\tmp\moc\release-shared-mt"),
    (Join-Path $qtRoot "src\tmp\obj\release-shared-mt"),
    (Join-Path $qtRoot "src\tmp\moc\release-shared-mt")
)
foreach ($directory in $qtBuildDirectories) {
    [void](New-Item -ItemType Directory -Path $directory -Force)
}

# qmake consumes this generated cache, so keep checkout-specific paths out of Git.
$qtPath = $qtRoot -replace '\\', '/'
$qmakeCache = @"
QMAKE_QT_VERSION_OVERRIDE=3
OBJECTS_DIR = tmp/obj/release-shared-mt
MOC_DIR = tmp/moc/release-shared-mt
DEFINES +=
INCLUDEPATH +=
sql-drivers +=
sql-plugins +=
styles += cde common compact interlace motifplus motif platinum sgi windows
style-plugins +=
imageformat-plugins +=
QT_PRODUCT=qt-free
CONFIG += enterprise nocrosscompiler rtti warn_off create_prl link_prl minimal-config small-config medium-config large-config full-config release shared thread no-exceptions no-incremental no-largefile no-gif no-tablet ipv6 zlib no-opengl sound precompile_header bigcodecs styles tools thread kernel widgets dialogs iconview workspace network canvas table xml opengl sql accessibility tablet sound png mng jpeg
QMAKESPEC=win32-msvc2005
QT_BUILD_TREE = $qtPath
QT_SOURCE_TREE = $qtPath
QT_INSTALL_PREFIX = $qtPath
QT_INSTALL_TRANSLATIONS = $qtPath/translations
QMAKE_LIBDIR_QT = $qtPath/lib
docs.path = $qtPath/doc
headers.path = $qtPath/include
plugins.path = $qtPath/plugins
libs.path = $qtPath/lib
bins.path = $qtPath/bin
data.path = $qtPath
translations.path = $qtPath/translations
CONFIG -=
CONFIG += shared thread release rtti
"@
[IO.File]::WriteAllText(
    (Join-Path $qtRoot ".qmake.cache"),
    $qmakeCache.TrimStart() + [Environment]::NewLine,
    [Text.Encoding]::ASCII)

$qmake = Join-Path $qtRoot "bin\qmake.exe"
if (-not (Test-Path -LiteralPath $qmake -PathType Leaf)) {
    Push-Location (Join-Path $qtRoot "qmake")
    try {
        & nmake.exe /NOLOGO
        if ($LASTEXITCODE -ne 0) {
            throw "The x64 Qt qmake build failed with exit code $LASTEXITCODE."
        }
    }
    finally {
        Pop-Location
    }
}

& $qmake -spec win32-msvc2005 -o (Join-Path $qtRoot "src\moc\Makefile") (Join-Path $qtRoot "src\moc\moc.pro")
if ($LASTEXITCODE -ne 0) {
    throw "qmake failed to generate the moc Makefile."
}

Push-Location (Join-Path $qtRoot "src\moc")
try {
    & nmake.exe /NOLOGO
    if ($LASTEXITCODE -ne 0) {
        throw "The x64 Qt moc build failed with exit code $LASTEXITCODE."
    }
}
finally {
    Pop-Location
}

& $qmake -spec win32-msvc2005 -o (Join-Path $qtRoot "src\Makefile") (Join-Path $qtRoot "src\qt.pro")
if ($LASTEXITCODE -ne 0) {
    throw "qmake failed to generate the Qt library Makefile."
}
& $qmake -spec win32-msvc2005 -o (Join-Path $qtRoot "src\Makefile.main") (Join-Path $qtRoot "src\qtmain.pro")
if ($LASTEXITCODE -ne 0) {
    throw "qmake failed to generate the qtmain Makefile."
}

Push-Location (Join-Path $qtRoot "src")
try {
    & nmake.exe /NOLOGO
    if ($LASTEXITCODE -ne 0) {
        throw "The x64 Qt library build failed with exit code $LASTEXITCODE."
    }
    & nmake.exe /NOLOGO /F Makefile.main
    if ($LASTEXITCODE -ne 0) {
        throw "The x64 Qt startup library build failed with exit code $LASTEXITCODE."
    }
}
finally {
    Pop-Location
}

$artifacts = @(
    $qmake,
    (Join-Path $qtRoot "bin\moc.exe"),
    (Join-Path $qtRoot "lib\qt-mt3.dll")
)
foreach ($path in $artifacts) {
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "Expected x64 Qt artifact is missing: $path"
    }
    if ((Get-PeMachine -Path $path) -ne 0x8664) {
        throw "Expected an x64 Qt artifact: $path"
    }
}

foreach ($path in @((Join-Path $qtRoot "lib\qt-mt3.lib"), (Join-Path $qtRoot "lib\qtmain.lib"))) {
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "Expected x64 Qt import library is missing: $path"
    }
}

Write-Host "Built local Qt 3 x64 runtime: $(Join-Path $qtRoot 'lib\qt-mt3.dll')"
