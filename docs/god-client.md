# God Client Builds

The God client supports both its legacy Win32 build and a self-contained `Release|x64` build. The x64 build uses the locally prepared Qt 3 source in `deps/qt3-win64-src`; it has no build-time or runtime dependency on another source checkout.

## Requirements

- Visual Studio or Build Tools with the C++ desktop workload and the `v145` toolset.
- Microsoft DirectX SDK (June 2010), exposed through `DXSDK_DIR` or installed in its default location.
- An existing God client data package for staging and world editing.

## x64 Build and Stage

Run from PowerShell:

```powershell
Set-Location E:\SWG\SWGSource\Source\client-tools
.\scripts\Build-X64GodClient.ps1 `
  -StagePath 'E:\SWG\SWGSource\Godclient v1.0 by Erusman'
```

The script builds local Qt 3, the God executable, and all three D3D9 renderer DLLs. It verifies each runtime artifact as x64 before optionally staging it. Staging backs up replaced files below `swg\exe\win32\.swgsource-build-backups`, keeps package-relative data paths, and writes `swgsource-godclient-runtime.json` with hashes for every deployed file.

Launch `swg\exe\win32\SwgGodClient_r.exe` with `swg\exe\win32` as its working directory. The local test account is station ID `1001` with password `local`.

The validated x64 editor loads the world, renders through `gl05_r.dll`, accepts editor/game focus input, and connects to the local Docker server. Legacy Perforce integration is disabled in x64, and the bundled `mss64.dll` compatibility runtime is silent.

## Legacy Win32 Build

Use `scripts\Build-GodClient.ps1` and `scripts\Stage-GodClient.ps1` when the original x86 Qt, Miles, and Mozilla runtime is required.
