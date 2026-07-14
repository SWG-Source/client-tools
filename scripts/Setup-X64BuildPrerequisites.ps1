[CmdletBinding()]
param(
    [ValidateSet("Build", "Runtime", "All")]
    [string]$Scope = "Build",

    [switch]$DownloadOnly,

    [switch]$Install,

    [switch]$ForceDownload,

    [string]$CachePath
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

if ($DownloadOnly -and $Install) {
    throw "Choose either -DownloadOnly or -Install, not both."
}
if ($ForceDownload -and -not ($DownloadOnly -or $Install)) {
    throw "-ForceDownload must be used with -DownloadOnly or -Install."
}

$repoRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..")).Path
$manifestPath = Join-Path $repoRoot "deps\build-prerequisites\manifest.json"
$testScript = Join-Path $PSScriptRoot "Test-X64BuildPrerequisites.ps1"
$manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json

if (-not $CachePath) {
    $CachePath = Join-Path $repoRoot ([string]$manifest.cachePath)
}
elseif (-not [IO.Path]::IsPathRooted($CachePath)) {
    $CachePath = Join-Path $repoRoot $CachePath
}
$CachePath = [IO.Path]::GetFullPath($CachePath)

function Test-IsAdministrator {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = [Security.Principal.WindowsPrincipal]::new($identity)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Assert-MicrosoftPackage {
    param(
        [Parameter(Mandatory)]$Package,
        [Parameter(Mandatory)][string]$Path
    )

    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "Downloaded package is missing: $Path"
    }

    $item = Get-Item -LiteralPath $Path
    if ($Package.bytes -and $item.Length -ne [int64]$Package.bytes) {
        throw "Unexpected size for $($Package.displayName): $($item.Length) bytes (expected $($Package.bytes))."
    }

    $hash = (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash
    if ($Package.sha256 -and $hash -ne [string]$Package.sha256) {
        throw "SHA-256 verification failed for $($Package.displayName). Expected $($Package.sha256), found $hash."
    }

    $signature = Get-AuthenticodeSignature -LiteralPath $Path
    if ($signature.Status -ne [Management.Automation.SignatureStatus]::Valid) {
        throw "Authenticode verification failed for $($Package.displayName): $($signature.StatusMessage)"
    }
    $signerSubject = $null
    if ($signature.SignerCertificate) {
        $signerSubject = $signature.SignerCertificate.Subject
    }
    if (-not $signerSubject -or $signerSubject -notlike "*$($Package.expectedPublisher)*") {
        throw "Unexpected signer for $($Package.displayName): $signerSubject"
    }

    Write-Host ("Verified {0}: {1}" -f $Package.displayName, $hash) -ForegroundColor Green
}

function Get-PackageFile {
    param([Parameter(Mandatory)]$Package)

    New-Item -ItemType Directory -Path $CachePath -Force | Out-Null
    $target = Join-Path $CachePath ([string]$Package.fileName)
    if ((Test-Path -LiteralPath $target -PathType Leaf) -and -not $ForceDownload) {
        Assert-MicrosoftPackage -Package $Package -Path $target
        return $target
    }

    $partialName = "{0}.download{1}" -f [IO.Path]::GetFileNameWithoutExtension($target), [IO.Path]::GetExtension($target)
    $partial = Join-Path $CachePath $partialName
    if (Test-Path -LiteralPath $partial) {
        Remove-Item -LiteralPath $partial -Force
    }

    Write-Host "Downloading $($Package.displayName)..."
    try {
        $curl = Get-Command curl.exe -ErrorAction SilentlyContinue
        if ($curl) {
            & $curl.Source --fail --location --retry 3 --retry-delay 2 --output $partial ([string]$Package.url)
            if ($LASTEXITCODE -ne 0) {
                throw "curl.exe failed with exit code $LASTEXITCODE."
            }
        }
        else {
            Invoke-WebRequest -UseBasicParsing -Uri ([string]$Package.url) -OutFile $partial
        }

        Assert-MicrosoftPackage -Package $Package -Path $partial
        Move-Item -LiteralPath $partial -Destination $target -Force
    }
    catch {
        if (Test-Path -LiteralPath $partial) {
            Remove-Item -LiteralPath $partial -Force
        }
        throw
    }

    return $target
}

function Assert-InstallerExitCode {
    param(
        [Parameter(Mandatory)][int]$ExitCode,
        [Parameter(Mandatory)][string]$DisplayName
    )

    if ($ExitCode -notin @(0, 1641, 3010)) {
        throw "$DisplayName setup failed with exit code $ExitCode."
    }
    if ($ExitCode -in @(1641, 3010)) {
        Write-Warning "$DisplayName requested a Windows restart. Restart before building."
    }
}

function Install-Package {
    param(
        [Parameter(Mandatory)]$Package,
        [Parameter(Mandatory)][string]$Path
    )

    switch ([string]$Package.installMode) {
        "visual-studio" {
            Write-Host "Installing the Visual Studio C++ build workload..."
            $arguments = @($Package.installArguments | ForEach-Object { [string]$_ })
            $process = Start-Process -FilePath $Path -ArgumentList $arguments -Wait -PassThru -WindowStyle Hidden
            Assert-InstallerExitCode -ExitCode $process.ExitCode -DisplayName ([string]$Package.displayName)
        }
        "interactive" {
            Write-Host "Opening $($Package.displayName) setup. Complete the Microsoft wizard to continue."
            $process = Start-Process -FilePath $Path -Wait -PassThru
            Assert-InstallerExitCode -ExitCode $process.ExitCode -DisplayName ([string]$Package.displayName)
        }
        "directx-redist" {
            $extractPath = Join-Path $CachePath "directx-runtime-june-2010"
            $dxSetup = Join-Path $extractPath "DXSETUP.exe"
            if (-not (Test-Path -LiteralPath $dxSetup -PathType Leaf) -or $ForceDownload) {
                New-Item -ItemType Directory -Path $extractPath -Force | Out-Null
                $extractArguments = @("/Q", ('/T:"{0}"' -f $extractPath))
                $extract = Start-Process -FilePath $Path -ArgumentList $extractArguments -Wait -PassThru -WindowStyle Hidden
                Assert-InstallerExitCode -ExitCode $extract.ExitCode -DisplayName "$($Package.displayName) extraction"
            }
            if (-not (Test-Path -LiteralPath $dxSetup -PathType Leaf)) {
                throw "DirectX runtime extraction did not create: $dxSetup"
            }
            $signature = Get-AuthenticodeSignature -LiteralPath $dxSetup
            $signerSubject = $null
            if ($signature.SignerCertificate) {
                $signerSubject = $signature.SignerCertificate.Subject
            }
            if ($signature.Status -ne [Management.Automation.SignatureStatus]::Valid -or $signerSubject -notlike "*Microsoft Corporation*") {
                throw "The extracted DXSETUP.exe does not have a valid Microsoft signature."
            }
            Write-Host "Installing the legacy DirectX runtime..."
            $process = Start-Process -FilePath $dxSetup -ArgumentList "/silent" -Wait -PassThru -WindowStyle Hidden
            Assert-InstallerExitCode -ExitCode $process.ExitCode -DisplayName ([string]$Package.displayName)
        }
        default {
            throw "Unknown install mode '$($Package.installMode)' for $($Package.displayName)."
        }
    }
}

function Get-SelectedPackages {
    $scopeName = $Scope.ToLowerInvariant()
    if ($Scope -eq "All") {
        return @($manifest.packages)
    }
    return @($manifest.packages | Where-Object { @($_.scopes) -contains $scopeName })
}

function Get-BuildStatus {
    return & $testScript -AllowMissing -Quiet -PassThru
}

if (-not ($DownloadOnly -or $Install)) {
    & $testScript
    return
}

$selectedPackages = @(Get-SelectedPackages)
if ($DownloadOnly) {
    foreach ($package in $selectedPackages) {
        [void](Get-PackageFile -Package $package)
    }
    Write-Host "Verified prerequisite cache: $CachePath"
    return
}

$status = Get-BuildStatus
$requiresInstallation = $false
foreach ($package in $selectedPackages) {
    switch ([string]$package.id) {
        "visual-studio-build-tools" { $requiresInstallation = $requiresInstallation -or -not $status.VisualStudio.Ready }
        "windows-10-sdk" { $requiresInstallation = $requiresInstallation -or -not $status.WindowsSdk.Ready }
        "directx-sdk-june-2010" { $requiresInstallation = $requiresInstallation -or -not $status.DirectXSdk.Ready }
        "directx-runtime-june-2010" { $requiresInstallation = $true }
    }
}
if ($requiresInstallation -and -not (Test-IsAdministrator)) {
    throw "Installation requires an elevated PowerShell window. Right-click PowerShell and choose Run as administrator, then rerun this command."
}

foreach ($package in $selectedPackages) {
    $needed = $true
    switch ([string]$package.id) {
        "visual-studio-build-tools" { $needed = -not $status.VisualStudio.Ready }
        "windows-10-sdk" { $needed = -not $status.WindowsSdk.Ready }
        "directx-sdk-june-2010" { $needed = -not $status.DirectXSdk.Ready }
        "directx-runtime-june-2010" { $needed = $true }
    }

    if (-not $needed) {
        Write-Host "Already installed: $($package.displayName)" -ForegroundColor Green
        continue
    }

    $path = Get-PackageFile -Package $package
    Install-Package -Package $package -Path $path
    if (@($package.scopes) -contains "build") {
        $status = Get-BuildStatus
    }
}

if ($Scope -in @("Build", "All")) {
    & $testScript
}
Write-Host "Prerequisite setup complete."
