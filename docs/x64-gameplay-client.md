# x64 Gameplay Client

The `Release|x64` gameplay client builds from this repository with modern MSBuild. The port is self-contained under `client-tools`; it does not reference another source checkout at build time or runtime.

## Prerequisites

- Visual Studio 2026 or Build Tools 2026 with MSBuild 18, Desktop development with C++, and the v145 x64 MSVC toolset.
- Windows 10 SDK 10.0.19041 or newer.
- Microsoft DirectX SDK (June 2010). The similarly named DirectX redistributable is runtime-only and does not contain the build headers or libraries.
- The x64 Visual C++ runtime and the legacy DirectX runtime. The stage script verifies `vcruntime140.dll` and `d3dx9_43.dll` in `System32`.

The required non-system libraries and runtime DLLs, including the x64 libjpeg-turbo headers and static library, are vendored in `deps/x64` and `mss64-stub`. No system-wide libjpeg-turbo installation is required.

Check or install the complete build profile from the repository root:

```powershell
.\scripts\Test-X64BuildPrerequisites.ps1
# Run this command from elevated PowerShell when anything is missing:
.\scripts\Setup-X64BuildPrerequisites.ps1 -Install
```

The setup script downloads Microsoft installers into the ignored `deps/source-cache/build-prerequisites` cache, verifies their signatures and pinned hashes, and installs only missing components. See [the prerequisite manifest guide](../deps/build-prerequisites/README.md) for direct links and offline-cache commands.

## Build

From the repository root:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Build-X64Client.ps1
```

The script validates the complete `x64-dx9-vanilla` build profile, locates MSBuild 18 with v145, builds `SwgClient` as `Release|x64`, and verifies that the client and all three D3D9 raster DLLs have the x64 PE machine type.

To select a Visual Studio installation or toolset explicitly:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Build-X64Client.ps1 `
  -VisualStudioRoot "C:\Program Files\Microsoft Visual Studio\18\Community" `
  -PlatformToolset v145
```

## Outputs

- `src/build/win32/x64/Release/SwgClient_r.exe`
- `src/compile/win32/Direct3d9/Release/gl05_r.dll`
- `src/compile/win32/Direct3d9_ffp/Release/gl06_r.dll`
- `src/compile/win32/Direct3d9_vsps/Release/gl07_r.dll`
- `src/compile/win32/DllExport/Release/DllExport.dll`

## Stage

Stage the built runtime into an existing client data directory:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Stage-X64Client.ps1 `
  -ClientRoot "E:\SWG\SWGSource\SWGSource Client v3.0"
```

Or build and stage in one command:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Build-X64Client.ps1 `
  -StagePath "E:\SWG\SWGSource\SWGSource Client v3.0"
```

Staging validates every copied PE as x64, backs up only replaced runtime files under `.x64-backups`, and writes `x64-runtime-manifest.json`. It also backs up and removes incompatible local x86 copies of system DLLs such as `dbghelp.dll`, allowing the x64 process to use `System32`. It does not change client configuration, login settings, TOCs, or TRE files.

## Current Limits

- `mss64.dll` is a no-op compatibility implementation because a licensed x64 Miles library is not present. Miles defaults to disabled on x64, so this build is silent. Set `disableMiles=false` in `[ClientAudio]` only after installing a compatible real x64 Miles runtime.
- Vivox, Bink, and the retired TCG/browser components do not have usable x64 runtimes in this tree. Keep voice chat and intro video disabled.
- The regular D3D9 gameplay client and x64 God client are validated. See [the God client guide](god-client.md) for its build and staging workflow.
