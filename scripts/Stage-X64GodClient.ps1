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
    [pscustomobject]@{ Source = Join-Path $repoRoot "src\build\win32\x64\$Configuration\SwgGodClient_r.exe"; Name = "SwgGodClient_r.exe" },
    [pscustomobject]@{ Source = Join-Path $repoRoot "src\build\win32\x64\$Configuration\gl05_r.dll"; Name = "gl05_r.dll" },
    [pscustomobject]@{ Source = Join-Path $repoRoot "src\build\win32\x64\$Configuration\gl06_r.dll"; Name = "gl06_r.dll" },
    [pscustomobject]@{ Source = Join-Path $repoRoot "src\build\win32\x64\$Configuration\gl07_r.dll"; Name = "gl07_r.dll" },
    [pscustomobject]@{ Source = Join-Path $repoRoot "src\build\win32\x64\$Configuration\DllExport.dll"; Name = "DllExport.dll" },
    [pscustomobject]@{ Source = Join-Path $repoRoot "deps\qt3-win64-src\lib\qt-mt3.dll"; Name = "qt-mt3.dll" },
    [pscustomobject]@{ Source = Join-Path $repoRoot "deps\x64\bin\SDL3.dll"; Name = "SDL3.dll" },
    [pscustomobject]@{ Source = Join-Path $repoRoot "mss64-stub\mss64.dll"; Name = "mss64.dll" },
    [pscustomobject]@{ Source = Join-Path $repoRoot "deps\x64\bin\libxml2.dll"; Name = "libxml2.dll" },
    [pscustomobject]@{ Source = Join-Path $repoRoot "deps\x64\bin\iconv-2.dll"; Name = "iconv-2.dll" },
    [pscustomobject]@{ Source = Join-Path $repoRoot "deps\x64\bin\z.dll"; Name = "z.dll" }
)
foreach ($file in $runtimeFiles) {
    if (-not (Test-Path -LiteralPath $file.Source -PathType Leaf)) {
        throw "Runtime source is missing. Build the x64 God client first: $($file.Source)"
    }
    if ((Get-PeMachine -Path $file.Source) -ne 0x8664) {
        throw "Refusing to stage a non-x64 runtime file: $($file.Source)"
    }
}

foreach ($path in @((Join-Path $env:SystemRoot "System32\d3dx9_43.dll"), (Join-Path $env:SystemRoot "System32\vcruntime140.dll"))) {
    if (-not (Test-Path -LiteralPath $path -PathType Leaf) -or (Get-PeMachine -Path $path) -ne 0x8664) {
        throw "Required x64 system runtime is missing or incompatible: $path"
    }
}

$localSystemDllNames = @("dbghelp.dll", "dbghelp_6.3.17.0.dll", "d3d9.dll", "d3dx9_43.dll", "ddraw.dll", "dinput8.dll", "vcruntime140.dll")
$incompatibleLocalPaths = @(
    $localSystemDllNames | ForEach-Object {
        $path = Join-Path $runtimeRoot $_
        if (Test-Path -LiteralPath $path -PathType Leaf) {
            if ((Get-PeMachine -Path $path) -ne 0x8664) {
                $path
            }
        }
    }
)

if (-not $PSCmdlet.ShouldProcess($runtimeRoot, "stage $Configuration x64 God client")) {
    return
}

$backupDirectory = $null
$newFiles = @($runtimeFiles | Where-Object { -not (Test-Path -LiteralPath (Join-Path $runtimeRoot $_.Name) -PathType Leaf) } | ForEach-Object Name)
if (-not $NoBackup) {
    $existingTargets = @(
        $runtimeFiles | ForEach-Object { Join-Path $runtimeRoot $_.Name } | Where-Object { Test-Path -LiteralPath $_ -PathType Leaf }
    )
    $existingTargets += @($clientConfigPath, $godConfigPath)
    if (Test-Path -LiteralPath $manifestPath -PathType Leaf) {
        $existingTargets += $manifestPath
    }
    $existingTargets += $incompatibleLocalPaths
    $existingTargets = @($existingTargets | Sort-Object -Unique)

    $backupDirectory = Join-Path $runtimeRoot (".swgsource-build-backups\{0}-x64" -f (Get-Date -Format "yyyyMMdd-HHmmss"))
    if (Test-Path -LiteralPath $backupDirectory) {
        $backupDirectory += "-$PID"
    }
    New-Item -ItemType Directory -Path $backupDirectory -Force | Out-Null
    foreach ($path in $existingTargets) {
        Copy-Item -LiteralPath $path -Destination (Join-Path $backupDirectory ([IO.Path]::GetFileName($path))) -Force
    }
}

foreach ($path in $incompatibleLocalPaths) {
    Remove-Item -LiteralPath $path -Force
}
foreach ($file in $runtimeFiles) {
    Copy-Item -LiteralPath $file.Source -Destination (Join-Path $runtimeRoot $file.Name) -Force
}

$utf8NoBom = [Text.UTF8Encoding]::new($false)
$clientConfig = [IO.File]::ReadAllText($clientConfigPath)
if ($clientConfig -notmatch '(?im)^\s*loginServerAddress\s*=') {
    throw "client.cfg does not define loginServerAddress: $clientConfigPath"
}
$clientConfig = [Text.RegularExpressions.Regex]::Replace($clientConfig, '(?im)^(\s*loginServerAddress\s*=\s*).+$', '${1}127.0.0.1')
[IO.File]::WriteAllText($clientConfigPath, $clientConfig, $utf8NoBom)

if (-not $KeepAbsoluteConfigPaths) {
    $godConfig = [IO.File]::ReadAllText($godConfigPath)
    $godConfig = [Text.RegularExpressions.Regex]::Replace($godConfig, '(?i)c:[\\/]+swg[\\/]+data', '../../data')
    $godConfig = [Text.RegularExpressions.Regex]::Replace($godConfig, '(?i)c:[\\/]+swg[\\/]+dsrc', '../../dsrc')
    [IO.File]::WriteAllText($godConfigPath, $godConfig, $utf8NoBom)
}

$gitCommit = (& git -C $repoRoot rev-parse HEAD).Trim()
$gitBranch = (& git -C $repoRoot branch --show-current).Trim()
$workingTreeDirty = @(& git -C $repoRoot status --porcelain).Count -gt 0
$stagedFiles = @(
    $runtimeFiles | ForEach-Object {
        $destination = Join-Path $runtimeRoot $_.Name
        [ordered]@{
            name = $_.Name
            bytes = (Get-Item -LiteralPath $destination).Length
            sha256 = (Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash
            machine = "x64"
        }
    }
)

$manifest = [ordered]@{
    formatVersion = 2
    generatedAtUtc = [DateTime]::UtcNow.ToString("o")
    configuration = $Configuration
    platform = "x64"
    sourceRepository = $repoRoot
    sourceCommit = $gitCommit
    sourceBranch = $gitBranch
    workingTreeDirty = $workingTreeDirty
    godClientRoot = $godClientRootPath
    runtimeRoot = $runtimeRoot
    inputBackend = "SDL 3.4.10 multi-device controller input"
    loginServerAddress = "127.0.0.1"
    relativeDataPaths = -not $KeepAbsoluteConfigPaths
    qtRuntime = "local Qt 3 Windows port, x64"
    audioBackend = "mss64 compatibility stub (silent)"
    perforceIntegration = "disabled in x64 build"
    backupDirectory = $backupDirectory
    newlyAddedFiles = $newFiles
    removedIncompatibleLocalFiles = @($incompatibleLocalPaths | ForEach-Object { [IO.Path]::GetFileName($_) })
    files = $stagedFiles
}
$json = $manifest | ConvertTo-Json -Depth 5
[IO.File]::WriteAllText($manifestPath, $json + [Environment]::NewLine, $utf8NoBom)

Write-Host "Staged $($runtimeFiles.Count) x64 God client runtime files to $runtimeRoot"
if ($backupDirectory) {
    Write-Host "Previous runtime and configuration files were backed up to $backupDirectory"
}
Write-Warning "The bundled mss64 compatibility DLL is silent, and the legacy Perforce integration is disabled in this x64 build."
