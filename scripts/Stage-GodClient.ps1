[CmdletBinding(SupportsShouldProcess)]
param(
    [Parameter(Mandatory)]
    [string]$GodClientRoot,

    [ValidateSet("Release")]
    [string]$Configuration = "Release",

    [switch]$NoBackup,

    [switch]$KeepAbsoluteConfigPaths
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

if (-not (Test-Path -LiteralPath $GodClientRoot -PathType Container)) {
    throw "GodClientRoot does not exist: $GodClientRoot"
}

$godClientRootPath = (Resolve-Path -LiteralPath $GodClientRoot).Path
$repoRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..")).Path
$swgRoot = Join-Path $godClientRootPath "swg"
$runtimeRoot = Join-Path $swgRoot "exe\win32"
$clientConfigPath = Join-Path $runtimeRoot "client.cfg"
$godConfigPath = Join-Path $runtimeRoot "SwgGodClient.cfg"
$manifestPath = Join-Path $runtimeRoot "swgsource-godclient-runtime.json"

$requiredDirectories = @(
    $runtimeRoot,
    (Join-Path $swgRoot "data\sku.0\sys.server\compiled\game"),
    (Join-Path $swgRoot "data\sku.0\sys.shared\compiled\game"),
    (Join-Path $swgRoot "data\sku.0\sys.client\compiled\game"),
    (Join-Path $swgRoot "dsrc\sku.0\sys.server\compiled\game"),
    (Join-Path $swgRoot "dsrc\sku.0\sys.shared\compiled\game")
)

foreach ($path in $requiredDirectories) {
    if (-not (Test-Path -LiteralPath $path -PathType Container)) {
        throw "God client package directory is missing: $path"
    }
}

foreach ($path in @($clientConfigPath, $godConfigPath)) {
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "God client configuration is missing: $path"
    }
}

$runtimeFiles = @(
    [pscustomobject]@{
        Source = Join-Path $repoRoot "src\compile\win32\SwgGodClient\$Configuration\SwgGodClient_r.exe"
        Name   = "SwgGodClient_r.exe"
    },
    [pscustomobject]@{
        Source = Join-Path $repoRoot "src\compile\win32\Direct3d9\$Configuration\gl05_r.dll"
        Name   = "gl05_r.dll"
    },
    [pscustomobject]@{
        Source = Join-Path $repoRoot "src\compile\win32\Direct3d9_ffp\$Configuration\gl06_r.dll"
        Name   = "gl06_r.dll"
    },
    [pscustomobject]@{
        Source = Join-Path $repoRoot "src\compile\win32\Direct3d9_vsps\$Configuration\gl07_r.dll"
        Name   = "gl07_r.dll"
    },
    [pscustomobject]@{
        Source = Join-Path $repoRoot "src\compile\win32\DllExport\$Configuration\DllExport.dll"
        Name   = "DllExport.dll"
    },
    [pscustomobject]@{
        Source = Join-Path $repoRoot "src\compile\win32\dpvs\$Configuration\dpvs.dll"
        Name   = "dpvs.dll"
    },
    [pscustomobject]@{
        Source = Join-Path $repoRoot "deps\win32\bin\msvcr71.dll"
        Name   = "msvcr71.dll"
    }
)

foreach ($file in $runtimeFiles) {
    if (-not (Test-Path -LiteralPath $file.Source -PathType Leaf)) {
        throw "Runtime source is missing. Build the God client first: $($file.Source)"
    }

    $machine = Get-PeMachine -Path $file.Source
    if ($machine -ne 0x014c) {
        throw ("Refusing to stage non-x86 PE 0x{0:x4}: {1}" -f $machine, $file.Source)
    }
}

$packageDependencies = @(
    "qt-mt334.dll",
    "Mss32.dll",
    "nspr4.dll",
    "xul.dll"
)

foreach ($name in $packageDependencies) {
    $path = Join-Path $runtimeRoot $name
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "Required God client runtime dependency is missing: $path"
    }

    if ((Get-PeMachine -Path $path) -ne 0x014c) {
        throw "Required God client runtime dependency is not x86: $path"
    }
}

if (-not $PSCmdlet.ShouldProcess($runtimeRoot, "stage $Configuration Win32 God client")) {
    return
}

$backupDirectory = $null
if (-not $NoBackup) {
    $existingTargets = @(
        $runtimeFiles |
            ForEach-Object { Join-Path $runtimeRoot $_.Name } |
            Where-Object { Test-Path -LiteralPath $_ -PathType Leaf }
    )
    $existingTargets += @($clientConfigPath, $godConfigPath)
    if (Test-Path -LiteralPath $manifestPath -PathType Leaf) {
        $existingTargets += $manifestPath
    }
    $existingTargets = @($existingTargets | Sort-Object -Unique)

    $backupDirectory = Join-Path $runtimeRoot (".swgsource-build-backups\{0}" -f (Get-Date -Format "yyyyMMdd-HHmmss"))
    if (Test-Path -LiteralPath $backupDirectory) {
        $backupDirectory += "-$PID"
    }

    New-Item -ItemType Directory -Path $backupDirectory -Force | Out-Null
    foreach ($path in $existingTargets) {
        Copy-Item -LiteralPath $path -Destination (Join-Path $backupDirectory ([IO.Path]::GetFileName($path))) -Force
    }
}

foreach ($file in $runtimeFiles) {
    Copy-Item -LiteralPath $file.Source -Destination (Join-Path $runtimeRoot $file.Name) -Force
}

$utf8NoBom = [Text.UTF8Encoding]::new($false)
$clientConfig = [IO.File]::ReadAllText($clientConfigPath)
if ($clientConfig -notmatch '(?im)^\s*loginServerAddress\s*=') {
    throw "client.cfg does not define loginServerAddress: $clientConfigPath"
}
$clientConfig = [Text.RegularExpressions.Regex]::Replace(
    $clientConfig,
    '(?im)^(\s*loginServerAddress\s*=\s*).+$',
    '${1}127.0.0.1'
)
[IO.File]::WriteAllText($clientConfigPath, $clientConfig, $utf8NoBom)

if (-not $KeepAbsoluteConfigPaths) {
    $godConfig = [IO.File]::ReadAllText($godConfigPath)
    $godConfig = [Text.RegularExpressions.Regex]::Replace(
        $godConfig,
        '(?i)c:[\\/]+swg[\\/]+data',
        '../../data'
    )
    $godConfig = [Text.RegularExpressions.Regex]::Replace(
        $godConfig,
        '(?i)c:[\\/]+swg[\\/]+dsrc',
        '../../dsrc'
    )
    $godConfig = [Text.RegularExpressions.Regex]::Replace(
        $godConfig,
        '(?m)^(\s*[A-Za-z][A-Za-z0-9]*),\s*"([^"]*)"\);\s*$',
        '$1="$2"'
    )
    [IO.File]::WriteAllText($godConfigPath, $godConfig, $utf8NoBom)
}

$gitCommit = (& git -C $repoRoot rev-parse HEAD).Trim()
$gitBranch = (& git -C $repoRoot branch --show-current).Trim()
$workingTreeDirty = @(& git -C $repoRoot status --porcelain).Count -gt 0

$stagedFiles = @(
    $runtimeFiles | ForEach-Object {
        $destination = Join-Path $runtimeRoot $_.Name
        [ordered]@{
            name   = $_.Name
            bytes  = (Get-Item -LiteralPath $destination).Length
            sha256 = (Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash
        }
    }
)

$manifest = [ordered]@{
    formatVersion      = 1
    generatedAtUtc     = [DateTime]::UtcNow.ToString("o")
    configuration      = $Configuration
    platform           = "Win32"
    architectureReason = "Qt 3.3.4 in the God client package is x86-only"
    sourceRepository   = $repoRoot
    sourceCommit       = $gitCommit
    sourceBranch       = $gitBranch
    workingTreeDirty   = $workingTreeDirty
    godClientRoot      = $godClientRootPath
    runtimeRoot        = $runtimeRoot
    loginServerAddress = "127.0.0.1"
    relativeDataPaths  = -not $KeepAbsoluteConfigPaths
    backupDirectory    = $backupDirectory
    files              = $stagedFiles
}

$json = $manifest | ConvertTo-Json -Depth 5
[IO.File]::WriteAllText($manifestPath, $json + [Environment]::NewLine, $utf8NoBom)

Write-Host "Staged $($runtimeFiles.Count) Win32 God client runtime files to $runtimeRoot"
if ($backupDirectory) {
    Write-Host "Previous runtime and configuration files were backed up to $backupDirectory"
}
Write-Host "God client data paths are relative to swg\exe\win32 and loginServerAddress is 127.0.0.1."
