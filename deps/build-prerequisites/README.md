# x64 DX9 Build Prerequisites

This directory is the checked-in dependency profile for the `x64-dx9-vanilla` client and God client. Large Microsoft installers are referenced by URL and downloaded into the ignored `deps/source-cache/build-prerequisites` directory. They are not committed to Git.

## One-command setup

Open an elevated PowerShell window at the repository root and run:

```powershell
.\scripts\Setup-X64BuildPrerequisites.ps1 -Install
```

The script installs Visual Studio Build Tools 2026 with MSBuild, the v145 x64 C++ tools, and recommended Windows SDK components. It only opens the standalone Windows SDK or DirectX SDK wizard when that prerequisite is still missing. Run the checker at any time without elevation:

```powershell
.\scripts\Test-X64BuildPrerequisites.ps1
```

To populate an offline cache without installing anything:

```powershell
.\scripts\Setup-X64BuildPrerequisites.ps1 -DownloadOnly -Scope All
```

To install the legacy DirectX runtime needed by staged gameplay and God clients:

```powershell
.\scripts\Setup-X64BuildPrerequisites.ps1 -Install -Scope Runtime
```

## Download references

| Package | Purpose | Official reference |
| --- | --- | --- |
| Visual Studio Build Tools 2026 | MSBuild 18, v145 x64 compiler, linker, librarian, and nmake | [stable Build Tools bootstrapper](https://aka.ms/vs/stable/vs_BuildTools.exe) |
| Windows 10 SDK | Desktop Windows headers, x64 import libraries, and resource tools | [Windows 10 SDK setup](https://go.microsoft.com/fwlink/?linkid=2311805) |
| DirectX SDK (June 2010) | Legacy D3DX headers and x64 libraries required at build time | [official SDK download](https://www.microsoft.com/en-us/download/details.aspx?id=6812) |
| DirectX End-User Runtimes (June 2010) | D3DX9 and other side-by-side runtime DLLs required to run the client | [official runtime download](https://www.microsoft.com/en-us/download/details.aspx?id=8109) |

The supplied `directx_Jun2010_redist.exe` link is the runtime package, not the developer SDK. It cannot provide `d3dx9.h` or the SDK libraries needed for compilation, so both DirectX packages are represented in the manifest.

The Build Tools and Windows SDK bootstrap links are mutable Microsoft channels, so the setup script validates their Microsoft Authenticode signatures instead of pinning a changing hash. The two immutable DirectX downloads have pinned size and SHA-256 values in `manifest.json`, and every downloaded executable must also have a valid Microsoft signature.

All other x64 headers, static libraries, compatibility libraries, runtime DLLs, and prepared Qt 3 source are already tracked under `deps/x64` and `deps/qt3-win64-src`. The legacy Miles files under `mss64-stub` are not used by the JUCE x64 profile. The installers directly under the repository's legacy `deps` root are Visual Studio 2013/Win32 artifacts and are not used by this x64 profile.
