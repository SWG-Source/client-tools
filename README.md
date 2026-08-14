# Client/Tools Repo
This repository contains the source for the SWG Client as well as the tools that support certain aspects of development.

## Build Instructions
The legacy Win32 projects were originally configured for **Visual Studio 2013**. The gameplay client can now be built as Win32 or x64 with both the Direct3D 9 and Direct3D 11 renderer plug-ins. SDL3 multi-controller input is enabled in both architectures. JUCE audio is the default build choice; the legacy Miles path remains selectable.

The project has 3 configurations for building the applications:
* **Release** which is the version intended for public dissemination and gameplay. You may recognize this as the `_r` in the client name `SwgClient_r.exe`. 
* **Optimized** which is similar to the release client but has additional options and displays in-game for testing and is ideal for Quality Assurance or Support related activities. For example, this configuration allows for additional options like targeting static world objects, printing object information in the user interface, and releasing the camera from player attachment for custom views.
* **Debug** which is a development client that has extra features for testing and extensive logging and reporting. This build isn't particularly useful for any present application.

`Release` is the primary validated configuration. `Optimized` and `Debug` remain available for development and may expose pre-existing tool or third-party dependency issues.

To build the client, find the `SwgClient` project in solution explorer and right click then select `Build`. Note that other projects may have similar names (like `ClientGame`) but these are shared across multiple tools. The actual game client you need for playing the game is the `SwgClient` project.

### Client Build Script

Check the build environment:

```powershell
.\scripts\Test-ClientBuildPrerequisites.ps1
```

The required environment is Visual Studio or Build Tools with the selected MSVC toolset, a Windows SDK, and the Microsoft DirectX SDK (June 2010). Architecture-specific SDL3 inputs and the x64 compatibility libraries are vendored under `deps`.

Build the complete Release matrix (Win32 and x64, with DX9 and DX11) using JUCE audio:

```powershell
.\scripts\Build-Client.ps1
```

Build a specific architecture and require a specific renderer output:

```powershell
.\scripts\Build-Client.ps1 -Architecture x86 -Renderer DX9
.\scripts\Build-Client.ps1 -Architecture x64 -Renderer DX11
```

Select the legacy audio path instead of the default JUCE backend:

```powershell
.\scripts\Build-Client.ps1 -Architecture All -AudioBackend Miles
```

`-Architecture` accepts `x86`, `x64`, or `All`; `-Renderer` accepts `DX9`, `DX11`, or `All`; and `-AudioBackend` accepts `Juce` or `Miles`. A gameplay-client solution build produces its renderer dependencies together, while `-Renderer` controls which output set the script requires and verifies. The script also checks every selected executable and renderer DLL for the expected PE machine type.

JUCE 8 modules are dual-licensed under AGPLv3 or the commercial JUCE license. Distributors must select and comply with an applicable JUCE licensing path; see `src/external/3rd/library/JUCE-8.0.14/LICENSE.md`. A Miles build requires a compatible licensed Miles runtime; the included x64 compatibility stubs satisfy the build but are not a full audio implementation.

SDL input is enabled by default and supports multiple independent controllers. See the [multi-controller input guide](docs/inputreborn.md) for runtime configuration and keymap compatibility.

## Source Style

First-party C and C++ use Allman braces with tab-based indentation. The repository `.clang-format` file is the formatting authority for new and changed first-party source. Vendored dependencies under `deps` and `src/external/3rd` retain their upstream formatting so they remain reviewable against their source releases.

Numeric values that encode a limit, capacity, interval, unit conversion, binary layout, or other policy must use a typed, descriptive `constexpr` instead of an untyped enum or an unexplained literal. Literal values may remain where they are intrinsic to an API, a mathematical identity, an indexed component, or a data table; non-obvious derivations should be documented beside the owning constant.

## Shared Files
Please note that certain projects and files are prepended with `shared` which means they are files that are used in both the game engine ([the `src` repository](https://github.com/swg-source/src)) and the client. There are many enums, for instance, that must match between the client and server or there may be crashes, errors, unintended functionality or some combination thereof. ***If you make changes to any of these shared files, you must make the changes both in the src and in client-tools.***

## Deprecated Components
Some specific features have been removed or disabled from the client as they are either no longer needed or outside the scope of the development work of SWG Source. Those removals include:
* The In-Game Web Browser (which uses libmozilla) and any UI elements or commands to activate it
* The Trading Card Game and any UI elements or commands to activate it
* The Customer Service "Help" Context Menu and the Bug Reporting Form, and any UI elements or commands to activate it
* Any references to Perforce, a version control solution that is no longer used.

## Documentation
We're currently working to compile more guides and developer documentation, but what is available can be found on our [SWG Source Wiki](https://github.com/swg-source/swg-main/wiki).

## History
This repository and code has undergone extensive renovations and refactoring since its release in 2013 and some history isn't included in GitHub. If you're looking for how these files were originally received from SOE without modification, see the [whitengold repository](https://github.com/swg-source/whitengold).

## Branches
* **master** - The primary development and release branch.
* **stdlib** - Current "work in progress" for building on Visual Studio 2015, which includes it's own, complete STL implementation.
* **wolfssl** - A test branch where Darth was trying to implement DTLS SSL to make the connection secure. Feel free to finish this implementation.

## Contributing and More Information
Contributions and improvements are welcome and encouraged, please submit a pull request. If you have any questions or are looking for more information and haven't already joined us in Discord, you can join [here](https://discord.gg/Va8e6n8). Please note that any changes to the client-tools that requires a rebuild of the SwgClient, will also mean a newly compiled client binary must be added to the [client-assets repository](https://github.com/swg-source/client-assets) so it can be shipped to end users.

## Additional Dependencies
Most of the development tools use the [Qt framework](https://www.qt.io/) to render their user interface. You may wish to install the [Qt VS Tools for Visual Studio](https://marketplace.visualstudio.com/items?itemName=TheQtCompany.QtVisualStudioTools-19123) to ease development. 

## Known Issues
* For the debug build, and possibly the optimized versions, you will get linker errors about libmozilla, and in release, possibly Vivox - if you alter the project settings you can disable this from killing the output of an exe, as libmozilla is only needed for the ingame browser.
* Other linker errors sometimes throw, you have to work on these case by case. Please pull request any changes you make.
* cmd.exe issues sometimes occur as SOE originally had the build setup copying files to a proper game bin directory. You can just remove these from projects that complain about them, just copy the output files manually.
* Plenty of warnings and sometimes even errors regarding deprecated libs happen. Fixes for these are case by case.
