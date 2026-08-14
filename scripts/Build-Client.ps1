[CmdletBinding()]
param(
    [ValidateSet("Release", "Optimized", "Debug")]
    [string]$Configuration = "Release",

    [ValidateSet("x86", "x64", "All")]
    [string]$Architecture = "All",

    [ValidateSet("DX9", "DX11", "All")]
    [string]$Renderer = "All",

    [ValidateSet("Juce", "Miles")]
    [string]$AudioBackend = "Juce",

    [string]$PlatformToolset = "v145",

    [string]$VisualStudioRoot,

    [ValidateRange(0, 128)]
    [int]$MaxCpuCount = 0
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

function Assert-Inputs {
    param(
        [Parameter(Mandatory)][string]$RepoRoot,
        [Parameter(Mandatory)][string]$Platform,
        [Parameter(Mandatory)][string]$SelectedAudioBackend
    )

    $dependencyRoot = if ($Platform -eq "x64") { "deps\x64" } else { "deps\win32" }
    $required = @(
        "$dependencyRoot\include\SDL3\SDL.h",
        "$dependencyRoot\lib\SDL3.lib",
        "$dependencyRoot\bin\SDL3.dll"
    )

    if ($Platform -eq "x64") {
        $required += @(
            "deps\x64\include\libjpeg-turbo\jpeglib.h",
            "deps\x64\lib\jpeg-static.lib",
            "deps\x64\lib\libxml2.lib",
            "deps\x64\lib\pcre.lib",
            "deps\x64\lib\dpvs.lib",
            "deps\x64\lib\libEverQuestTCG.lib",
            "deps\x64\lib\vivoxSharedWrapper.lib",
            "deps\x64\lib\swg-stubs.lib"
        )
    }

    if ($SelectedAudioBackend -eq "Juce") {
        $required += "src\external\3rd\library\JUCE-8.0.14\modules\juce_audio_devices\juce_audio_devices.cpp"
    }
    elseif ($Platform -eq "x64") {
        $required += @(
            "mss64-stub\mss64.lib",
            "mss64-stub\mss64.dll"
        )
    }

    foreach ($relativePath in $required) {
        $path = Join-Path $RepoRoot $relativePath
        if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
            throw "Required build input is missing: $path"
        }
    }
}

function Get-ExpectedArtifacts {
    param(
        [Parameter(Mandatory)][string]$Platform,
        [Parameter(Mandatory)][string]$SelectedConfiguration,
        [Parameter(Mandatory)][string]$SelectedRenderer,
        [Parameter(Mandatory)][string]$Suffix
    )

    if ($Platform -eq "x64") {
        $outputRoot = "src\build\win32\x64\$SelectedConfiguration"
        $artifacts = @("$outputRoot\SwgClient_$Suffix.exe")
        if ($SelectedRenderer -eq "DX9" -or $SelectedRenderer -eq "All") {
            $artifacts += @(
                "$outputRoot\gl05_$Suffix.dll",
                "$outputRoot\gl06_$Suffix.dll",
                "$outputRoot\gl07_$Suffix.dll"
            )
        }
        if ($SelectedRenderer -eq "DX11" -or $SelectedRenderer -eq "All") {
            $artifacts += "$outputRoot\gl11_$Suffix.dll"
        }
        return $artifacts
    }

    $artifacts = @("src\compile\win32\SwgClient\$SelectedConfiguration\SwgClient_$Suffix.exe")
    if ($SelectedRenderer -eq "DX9" -or $SelectedRenderer -eq "All") {
        $artifacts += @(
            "src\compile\win32\Direct3d9\$SelectedConfiguration\gl05_$Suffix.dll",
            "src\compile\win32\Direct3d9_ffp\$SelectedConfiguration\gl06_$Suffix.dll",
            "src\compile\win32\Direct3d9_vsps\$SelectedConfiguration\gl07_$Suffix.dll"
        )
    }
    if ($SelectedRenderer -eq "DX11" -or $SelectedRenderer -eq "All") {
        $artifacts += "src\compile\win32\Direct3d11\$SelectedConfiguration\gl11_$Suffix.dll"
    }
    return $artifacts
}

$repoRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..")).Path
$solution = Join-Path $repoRoot "src\build\win32\swg.sln"
$prerequisites = & (Join-Path $PSScriptRoot "Test-ClientBuildPrerequisites.ps1") `
    -PlatformToolset $PlatformToolset `
    -VisualStudioRoot $VisualStudioRoot `
    -Quiet `
    -PassThru

$msbuild = $prerequisites.VisualStudio.MSBuildPath
$env:DXSDK_DIR = $prerequisites.DirectXSdk.Root.TrimEnd("\") + "\"

$platforms = switch ($Architecture) {
    "x86" { @("Win32") }
    "x64" { @("x64") }
    default { @("Win32", "x64") }
}

$suffix = @{
    Release   = "r"
    Optimized = "o"
    Debug     = "d"
}[$Configuration]

foreach ($platform in $platforms) {
    Assert-Inputs -RepoRoot $repoRoot -Platform $platform -SelectedAudioBackend $AudioBackend

    $arguments = @(
        $solution,
        "/t:SwgClient",
        "/p:Configuration=$Configuration",
        "/p:Platform=$platform",
        "/p:PlatformToolset=$PlatformToolset",
        "/p:SwgAudioBackend=$AudioBackend",
        "/nr:false",
        "/v:minimal"
    )
    if ($MaxCpuCount -gt 0) {
        $arguments += "/m:$MaxCpuCount"
    }
    else {
        $arguments += "/m"
    }

    Write-Host "MSBuild: $msbuild"
    Write-Host "Solution: $solution"
    Write-Host "Build: $Configuration|$platform; renderers=$Renderer; audio=$AudioBackend; toolset=$PlatformToolset"

    & $msbuild @arguments
    if ($LASTEXITCODE -ne 0) {
        throw "The $Configuration|$platform client build failed with exit code $LASTEXITCODE."
    }

    $expectedMachine = if ($platform -eq "x64") { 0x8664 } else { 0x014c }
    $artifacts = Get-ExpectedArtifacts `
        -Platform $platform `
        -SelectedConfiguration $Configuration `
        -SelectedRenderer $Renderer `
        -Suffix $suffix

    foreach ($relativePath in $artifacts) {
        $path = Join-Path $repoRoot $relativePath
        if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
            throw "Expected build artifact is missing: $path"
        }

        $machine = Get-PeMachine -Path $path
        if ($machine -ne $expectedMachine) {
            throw ("Expected PE machine 0x{0:x4}, found 0x{1:x4}: {2}" -f $expectedMachine, $machine, $path)
        }

        $item = Get-Item -LiteralPath $path
        $hash = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash
        Write-Host ("{0,-5} {1,10:N0} bytes  {2}  {3}" -f $platform, $item.Length, $hash, $path)
    }
}
