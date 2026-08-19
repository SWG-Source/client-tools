<#
.SYNOPSIS
Contract gate for the GetEngineHookPoints advertise surface (both platforms).

.DESCRIPTION
Loads a staged SwgClient exe AS AN IMAGE (LoadLibraryEx DONT_RESOLVE_DLL_REFERENCES,
no boot, no game data) and calls GetEngineHookPoints exactly the way the injected
consumer does -- the pre-CRT read path the 2026-06-25 static-init race fix was
engineered for: the whole fill is pure address arithmetic (no heap, no TLS, no CRT,
no imports), so calling it in a foreign process without running the exe is safe by
the same argument that makes the remote-thread early read safe.

Checks, per exe (mirrors engine_verifyNoNullNoDup, which only runs on a Debug boot):
  1. the export resolves by NAME (the consumer's binding);
  2. version == ENGINE_HOOKPOINTS_VERSION parsed from engine_hookpoints.h;
  3. row count == the required-name count parsed from engine_hookpoints.inc;
  4. zero null addresses (a null row = a dyn-fill/real-entry extraction failure --
     on x64 that includes any MI real-entry whose delta != 0);
  5. zero duplicate names;
  6. name-set equality vs the .inc, both directions (nothing missing, no extras).

What it CANNOT check: row semantics (calling a row needs an installed engine) and
detour viability (consumer-side). It is a table/contract gate, not a boot smoke.

.USAGE
  powershell -File tools\hookpoints-probe\Probe-HookPoints.ps1
      probes stage\SwgClient_r.exe AND stage-x64\SwgClient_r.exe (skips a missing
      one with a note). A probe whose exe arch differs from the current process is
      re-run automatically under the matching PowerShell host (SysWOW64 / System32).
  powershell -File ...\Probe-HookPoints.ps1 -ExePath <path-to-SwgClient exe>
      probes one exe. Exit code 0 = all probed exes PASS, 1 = any FAIL.

Consumers (SWG-Toolkit) can run this against any staged build to verify the
surface they are about to bind, before injecting anything.
#>
[CmdletBinding()]
param(
	[string] $ExePath,
	[string] $RepoRoot
)

$ErrorActionPreference = 'Stop'

if (-not $RepoRoot)
{
	$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
}

$script:HookpointsH   = Join-Path $RepoRoot 'src\game\client\application\SwgClient\src\shared\engine_hookpoints.h'
$script:HookpointsInc = Join-Path $RepoRoot 'src\game\client\application\SwgClient\src\shared\engine_hookpoints.inc'

function Get-ExpectedVersion
{
	$m = Select-String -Path $script:HookpointsH -Pattern '#define\s+ENGINE_HOOKPOINTS_VERSION\s+(\d+)' | Select-Object -First 1
	if (-not $m) { throw "ENGINE_HOOKPOINTS_VERSION not found in $script:HookpointsH" }
	return [int]$m.Matches[0].Groups[1].Value
}

function Get-RequiredNames
{
	# comment lines are stripped first -- the .inc header documents the macro shape
	# as ENGINE_HOOKPOINT(group, name) inside a comment, which the regex would match
	$names = @()
	foreach ($line in (Get-Content $script:HookpointsInc))
	{
		$code = ($line -replace '//.*$', '')
		$m = [regex]::Match($code, 'ENGINE_HOOKPOINT\(\s*(\w+)\s*,\s*(\w+)\s*\)')
		if ($m.Success)
		{
			$names += ($m.Groups[1].Value + '::' + $m.Groups[2].Value)
		}
	}
	if ($names.Count -eq 0) { throw "no ENGINE_HOOKPOINT rows parsed from $script:HookpointsInc" }
	return $names
}

# PE machine field: 0x8664 = x64, 0x014c = i386.
function Get-PeMachine([string] $path)
{
	$fs = [IO.File]::OpenRead($path)
	try
	{
		$br = New-Object IO.BinaryReader($fs)
		$fs.Position = 0x3C
		$elfanew = $br.ReadInt32()
		$fs.Position = $elfanew + 4
		return $br.ReadUInt16()
	}
	finally
	{
		$fs.Dispose()
	}
}

function Invoke-TableProbe([string] $exe, [int] $expectedVersion, [string[]] $requiredNames)
{
	if (-not ('HookProbe.Native' -as [type]))
	{
		Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
namespace HookProbe {
	public static class Native {
		[DllImport("kernel32", SetLastError=true, CharSet=CharSet.Ansi)]
		public static extern IntPtr LoadLibraryEx(string path, IntPtr hFile, uint flags);
		[DllImport("kernel32", SetLastError=true, CharSet=CharSet.Ansi)]
		public static extern IntPtr GetProcAddress(IntPtr h, string name);
		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		public delegate IntPtr GetHooksDelegate();
	}
}
'@
	}

	$failures = @()

	$h = [HookProbe.Native]::LoadLibraryEx($exe, [IntPtr]::Zero, 0x1)   # DONT_RESOLVE_DLL_REFERENCES
	if ($h -eq [IntPtr]::Zero)
	{
		$failures += "LoadLibraryEx failed (Win32 error $([Runtime.InteropServices.Marshal]::GetLastWin32Error()))"
		return $failures
	}

	$p = [HookProbe.Native]::GetProcAddress($h, 'GetEngineHookPoints')
	if ($p -eq [IntPtr]::Zero)
	{
		$failures += 'export GetEngineHookPoints did not resolve by name'
		return $failures
	}

	$fn  = [Runtime.InteropServices.Marshal]::GetDelegateForFunctionPointer($p, [HookProbe.Native+GetHooksDelegate])
	$tbl = $fn.Invoke()
	if ($tbl -eq [IntPtr]::Zero)
	{
		$failures += 'GetEngineHookPoints returned null'
		return $failures
	}

	# EngineHookPoints: { uint version; uint count; const EngineHookPoint* entries; }
	# entries offset = 8 on BOTH platforms (two uints, then pointer alignment <= 8).
	$version = [Runtime.InteropServices.Marshal]::ReadInt32($tbl, 0)
	$count   = [Runtime.InteropServices.Marshal]::ReadInt32($tbl, 4)
	$entries = [Runtime.InteropServices.Marshal]::ReadIntPtr($tbl, 8)
	$stride  = 2 * [IntPtr]::Size   # EngineHookPoint: { const char* name; void* addr; }

	if ($version -ne $expectedVersion) { $failures += "version $version != expected $expectedVersion (engine_hookpoints.h)" }
	if ($count -ne $requiredNames.Count) { $failures += "count $count != required $($requiredNames.Count) (engine_hookpoints.inc)" }

	$seen = @{}
	$nulls = 0
	for ($i = 0; $i -lt $count; $i++)
	{
		$namePtr = [Runtime.InteropServices.Marshal]::ReadIntPtr($entries, $i * $stride)
		$addr    = [Runtime.InteropServices.Marshal]::ReadIntPtr($entries, $i * $stride + [IntPtr]::Size)
		$name    = [Runtime.InteropServices.Marshal]::PtrToStringAnsi($namePtr)
		if ($null -eq $name) { $name = "(null-name row $i)" }
		if ($addr -eq [IntPtr]::Zero) { $nulls++; $failures += "NULL addr: $name" }
		if ($seen.ContainsKey($name)) { $failures += "DUPLICATE name: $name" } else { $seen[$name] = $true }
	}

	foreach ($required in $requiredNames)
	{
		if (-not $seen.ContainsKey($required)) { $failures += "MISSING required name: $required" }
	}
	foreach ($name in $seen.Keys)
	{
		if ($requiredNames -notcontains $name) { $failures += "EXTRA name not in .inc: $name" }
	}

	Write-Host ("  version={0} count={1} nulls={2} uniqueNames={3}" -f $version, $count, $nulls, $seen.Count)
	return $failures
}

function Invoke-ExeProbe([string] $exe, [int] $expectedVersion, [string[]] $requiredNames)
{
	$machine = Get-PeMachine $exe
	$exeIs64 = ($machine -eq 0x8664)
	$procIs64 = [Environment]::Is64BitProcess

	$archLabel = 'Win32'
	if ($exeIs64) { $archLabel = 'x64' }
	Write-Host ("[{0}] {1}" -f $archLabel, $exe)

	if ($exeIs64 -eq $procIs64)
	{
		$failures = Invoke-TableProbe $exe $expectedVersion $requiredNames
		if ($failures.Count -eq 0)
		{
			Write-Host '  PASS'
			return $true
		}
		foreach ($f in $failures) { Write-Host "  FAIL: $f" }
		return $false
	}

	# arch mismatch: re-run THIS script single-exe under the matching PowerShell host
	if ($exeIs64)
	{
		$psHost = Join-Path $env:SystemRoot 'sysnative\WindowsPowerShell\v1.0\powershell.exe'   # from a 32-bit process
		if (-not (Test-Path $psHost)) { $psHost = Join-Path $env:SystemRoot 'System32\WindowsPowerShell\v1.0\powershell.exe' }
	}
	else
	{
		$psHost = Join-Path $env:SystemRoot 'SysWOW64\WindowsPowerShell\v1.0\powershell.exe'
	}
	$childOut = & $psHost -NoProfile -ExecutionPolicy Bypass -File $PSCommandPath -ExePath $exe -RepoRoot $RepoRoot
	$childExit = $LASTEXITCODE
	foreach ($line in $childOut)
	{
		# the child re-prints its own "[arch] exe" header -- drop it, we already printed ours
		if ("$line" -notmatch '^\[(Win32|x64)\]') { Write-Host $line }
	}
	return ($childExit -eq 0)
}

$expectedVersion = Get-ExpectedVersion
$requiredNames   = Get-RequiredNames

if ($ExePath)
{
	$ok = Invoke-ExeProbe (Resolve-Path $ExePath).Path $expectedVersion $requiredNames
	if ($ok) { exit 0 } else { exit 1 }
}

$targets = @(
	(Join-Path $RepoRoot 'stage\SwgClient_r.exe'),
	(Join-Path $RepoRoot 'stage-x64\SwgClient_r.exe')
)

$allOk = $true
foreach ($t in $targets)
{
	if (-not (Test-Path $t))
	{
		Write-Host "[skip] $t (not staged)"
		continue
	}
	$ok = Invoke-ExeProbe $t $expectedVersion $requiredNames
	if (-not $ok) { $allOk = $false }
}

if ($allOk)
{
	Write-Host 'ALL PROBED EXES PASS'
	exit 0
}
Write-Host 'PROBE FAILURES PRESENT'
exit 1
