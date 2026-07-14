[CmdletBinding()]
param(
    [string]$PlatformToolset = "v145",

    [string]$VisualStudioRoot,

    [switch]$AllowMissing,

    [switch]$Quiet,

    [switch]$PassThru
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..")).Path
$manifestPath = Join-Path $repoRoot "deps\build-prerequisites\manifest.json"
if (-not (Test-Path -LiteralPath $manifestPath -PathType Leaf)) {
    throw "The x64 prerequisite manifest is missing: $manifestPath"
}
$manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json

function Test-VisualStudioInstallation {
    param(
        [Parameter(Mandatory)][string]$Root,
        [string]$Version
    )

    $normalizedRoot = [IO.Path]::GetFullPath($Root.Trim().Trim([char]34).TrimEnd("\"))
    $msbuild = Join-Path $normalizedRoot "MSBuild\Current\Bin\MSBuild.exe"
    $devShell = Join-Path $normalizedRoot "Common7\Tools\Launch-VsDevShell.ps1"
    $vcTargetsRoot = Join-Path $normalizedRoot "MSBuild\Microsoft\VC"
    $compilerPattern = Join-Path $normalizedRoot "VC\Tools\MSVC\*\bin\Hostx64\x64\cl.exe"

    $toolsetFiles = @()
    if (Test-Path -LiteralPath $vcTargetsRoot -PathType Container) {
        $toolsetSuffix = "Platforms\x64\PlatformToolsets\$PlatformToolset\Toolset.props"
        $toolsetFiles = @(
            Get-ChildItem -LiteralPath $vcTargetsRoot -Recurse -Filter "Toolset.props" -File -ErrorAction SilentlyContinue |
                Where-Object { $_.FullName.EndsWith($toolsetSuffix, [StringComparison]::OrdinalIgnoreCase) }
        )
    }

    $compilers = @(Get-ChildItem -Path $compilerPattern -File -ErrorAction SilentlyContinue | Sort-Object FullName -Descending)
    $compiler = $null
    if ($compilers.Count -gt 0) {
        $compiler = $compilers[0].FullName
    }

    $compilerDirectory = $null
    if ($compiler) {
        $compilerDirectory = Split-Path -Parent $compiler
    }
    $librarian = $null
    $nmake = $null
    if ($compilerDirectory) {
        $librarian = Join-Path $compilerDirectory "lib.exe"
        $nmake = Join-Path $compilerDirectory "nmake.exe"
    }

    $ready = (
        (Test-Path -LiteralPath $msbuild -PathType Leaf) -and
        (Test-Path -LiteralPath $devShell -PathType Leaf) -and
        $toolsetFiles.Count -gt 0 -and
        $compiler -and
        (Test-Path -LiteralPath $librarian -PathType Leaf) -and
        (Test-Path -LiteralPath $nmake -PathType Leaf)
    )

    [pscustomobject]@{
        Ready = [bool]$ready
        Root = $normalizedRoot
        Version = $Version
        MSBuildPath = $msbuild
        DevShellPath = $devShell
        CompilerPath = $compiler
        Toolset = $PlatformToolset
    }
}

function Get-VisualStudioStatus {
    if ($VisualStudioRoot) {
        if (-not (Test-Path -LiteralPath $VisualStudioRoot -PathType Container)) {
            return [pscustomobject]@{
                Ready = $false
                Root = $VisualStudioRoot
                Version = $null
                MSBuildPath = $null
                DevShellPath = $null
                CompilerPath = $null
                Toolset = $PlatformToolset
            }
        }
        return Test-VisualStudioInstallation -Root $VisualStudioRoot
    }

    $roots = @()
    $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path -LiteralPath $vswhere -PathType Leaf) {
        $arguments = @(
            "-all",
            "-products", "*",
            "-version", [string]$manifest.visualStudio.versionRange,
            "-requires"
        )
        $arguments += @($manifest.visualStudio.requiredComponents)
        $arguments += @("-format", "json", "-utf8")

        $json = (& $vswhere @arguments 2>$null) -join [Environment]::NewLine
        if ($LASTEXITCODE -eq 0 -and $json) {
            $instances = @($json | ConvertFrom-Json | Sort-Object { [version]$_.installationVersion } -Descending)
            foreach ($instance in $instances) {
                $roots += [pscustomobject]@{
                    Root = [string]$instance.installationPath
                    Version = [string]$instance.installationVersion
                }
            }
        }
    }

    $fallbackRoots = @(
        (Join-Path $env:ProgramFiles "Microsoft Visual Studio\18\Community"),
        (Join-Path $env:ProgramFiles "Microsoft Visual Studio\18\Professional"),
        (Join-Path $env:ProgramFiles "Microsoft Visual Studio\18\Enterprise"),
        (Join-Path $env:ProgramFiles "Microsoft Visual Studio\18\BuildTools"),
        (Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\18\BuildTools")
    )
    foreach ($root in $fallbackRoots) {
        if ($root -and (Test-Path -LiteralPath $root -PathType Container)) {
            $roots += [pscustomobject]@{ Root = $root; Version = $null }
        }
    }

    $seen = @{}
    foreach ($candidate in $roots) {
        $key = [IO.Path]::GetFullPath($candidate.Root).ToLowerInvariant()
        if ($seen.ContainsKey($key)) {
            continue
        }
        $seen[$key] = $true

        $result = Test-VisualStudioInstallation -Root $candidate.Root -Version $candidate.Version
        if ($result.Ready) {
            return $result
        }
    }

    [pscustomobject]@{
        Ready = $false
        Root = $null
        Version = $null
        MSBuildPath = $null
        DevShellPath = $null
        CompilerPath = $null
        Toolset = $PlatformToolset
    }
}

function Get-WindowsSdkStatus {
    $minimumVersion = [version][string]$manifest.windowsSdk.minimumVersion
    $roots = @()
    try {
        $installedRoots = Get-ItemProperty -LiteralPath "HKLM:\SOFTWARE\Microsoft\Windows Kits\Installed Roots" -ErrorAction Stop
        if ($installedRoots.KitsRoot10) {
            $roots += [string]$installedRoots.KitsRoot10
        }
    }
    catch {
    }
    $roots += Join-Path ${env:ProgramFiles(x86)} "Windows Kits\10"

    $candidates = @()
    foreach ($root in @($roots | Select-Object -Unique)) {
        $includeRoot = Join-Path $root "Include"
        if (-not (Test-Path -LiteralPath $includeRoot -PathType Container)) {
            continue
        }
        foreach ($directory in (Get-ChildItem -LiteralPath $includeRoot -Directory -ErrorAction SilentlyContinue)) {
            $version = $null
            if (-not [version]::TryParse($directory.Name, [ref]$version) -or $version -lt $minimumVersion) {
                continue
            }
            $windowsHeader = Join-Path $directory.FullName "um\Windows.h"
            $sharedHeader = Join-Path $directory.FullName "shared\sdkddkver.h"
            $kernelLibrary = Join-Path $root "Lib\$($directory.Name)\um\x64\kernel32.lib"
            if (
                (Test-Path -LiteralPath $windowsHeader -PathType Leaf) -and
                (Test-Path -LiteralPath $sharedHeader -PathType Leaf) -and
                (Test-Path -LiteralPath $kernelLibrary -PathType Leaf)
            ) {
                $candidates += [pscustomobject]@{
                    Ready = $true
                    Root = [IO.Path]::GetFullPath($root)
                    Version = $version
                }
            }
        }
    }

    if ($candidates.Count -gt 0) {
        return $candidates | Sort-Object Version -Descending | Select-Object -First 1
    }

    [pscustomobject]@{
        Ready = $false
        Root = $null
        Version = $null
    }
}

function Get-DirectXSdkStatus {
    $roots = @()
    if ($env:DXSDK_DIR) {
        $roots += $env:DXSDK_DIR
    }
    $roots += Join-Path ${env:ProgramFiles(x86)} "Microsoft DirectX SDK (June 2010)"

    foreach ($root in @($roots | Where-Object { $_ } | Select-Object -Unique)) {
        $normalizedRoot = [IO.Path]::GetFullPath($root.Trim().Trim([char]34).TrimEnd("\"))
        $requiredFiles = @(
            (Join-Path $normalizedRoot "Include\d3dx9.h"),
            (Join-Path $normalizedRoot "Include\dinput.h"),
            (Join-Path $normalizedRoot "Lib\x64\d3dx9.lib")
        )
        $missing = @($requiredFiles | Where-Object { -not (Test-Path -LiteralPath $_ -PathType Leaf) })
        if ($missing.Count -eq 0) {
            return [pscustomobject]@{
                Ready = $true
                Root = $normalizedRoot
                Version = "9.29.1962.0"
            }
        }
    }

    [pscustomobject]@{
        Ready = $false
        Root = $null
        Version = $null
    }
}

function Get-VendoredInputStatus {
    $missing = @()
    foreach ($relativePath in @($manifest.vendoredInputs)) {
        $path = Join-Path $repoRoot ([string]$relativePath)
        if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
            $missing += [string]$relativePath
        }
    }

    [pscustomobject]@{
        Ready = ($missing.Count -eq 0)
        Missing = $missing
    }
}

function Write-PrerequisiteStatus {
    param(
        [Parameter(Mandatory)][bool]$Ready,
        [Parameter(Mandatory)][string]$Name,
        [string]$Detail
    )

    $prefix = "MISSING"
    $color = "Yellow"
    if ($Ready) {
        $prefix = "OK"
        $color = "Green"
    }
    if ($Detail) {
        Write-Host ("[{0}] {1}: {2}" -f $prefix, $Name, $Detail) -ForegroundColor $color
    }
    else {
        Write-Host ("[{0}] {1}" -f $prefix, $Name) -ForegroundColor $color
    }
}

$visualStudio = Get-VisualStudioStatus
$windowsSdk = Get-WindowsSdkStatus
$directXSdk = Get-DirectXSdkStatus
$vendoredInputs = Get-VendoredInputStatus

$missing = @()
if (-not $visualStudio.Ready) {
    $missing += "Visual Studio Build Tools 2026 with MSBuild and the $PlatformToolset x64 C++ toolset"
}
if (-not $windowsSdk.Ready) {
    $missing += "Windows 10 SDK $($manifest.windowsSdk.minimumVersion) or newer"
}
if (-not $directXSdk.Ready) {
    $missing += "DirectX SDK (June 2010) headers and x64 libraries"
}
if (-not $vendoredInputs.Ready) {
    $missing += "vendored repository inputs: $($vendoredInputs.Missing -join ', ')"
}

$status = [pscustomobject]@{
    Profile = [string]$manifest.profile
    Ready = ($missing.Count -eq 0)
    VisualStudio = $visualStudio
    WindowsSdk = $windowsSdk
    DirectXSdk = $directXSdk
    VendoredInputs = $vendoredInputs
    Missing = $missing
}

if (-not $Quiet) {
    Write-Host "x64 build prerequisite profile: $($status.Profile)"
    Write-PrerequisiteStatus -Ready $visualStudio.Ready -Name "Visual Studio and $PlatformToolset" -Detail $visualStudio.Root
    Write-PrerequisiteStatus -Ready $windowsSdk.Ready -Name "Windows SDK" -Detail ([string]$windowsSdk.Version)
    Write-PrerequisiteStatus -Ready $directXSdk.Ready -Name "DirectX SDK (June 2010)" -Detail $directXSdk.Root
    Write-PrerequisiteStatus -Ready $vendoredInputs.Ready -Name "Vendored x64 dependencies"
}

if ($PassThru) {
    Write-Output $status
}

if (-not $AllowMissing -and -not $status.Ready) {
    throw ("x64 build prerequisites are incomplete: {0}. Run from elevated PowerShell: .\scripts\Setup-X64BuildPrerequisites.ps1 -Install" -f ($missing -join "; "))
}
