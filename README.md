# Client/Tools Repo
This repository contains the source for the SWG Client as well as the tools that support certain aspects of development.

## Build Instructions
This repository is configured for use with Visual Studio using msbuild/solutions and is currently ONLY compatible with **Visual Studio 2013**. To setup your environment, acquire the community or professional edition (NOT the express edition) of Visual Studio 2013 and clone this repository to your machine. Open Visual Studio 2013 and use `File > Open > Project/Solution` and navigate to the path where you cloned the repository then select the solution file located at `src/build/win32/swg.sln`. Please note that all SWG applications are only compatible with win32.

The project has 3 configurations for building the applications:
* **Release** which is the version intended for public dissemination and gameplay. You may recognize this as the `_r` in the client name `SwgClient_r.exe`. 
* **Optimized** which is similar to the release client but has additional options and displays in-game for testing and is ideal for Quality Assurance or Support related activities. For example, this configuration allows for additional options like targeting static world objects, printing object information in the user interface, and releasing the camera from player attachment for custom views.
* **Debug** which is a development client that has extra features for testing and extensive logging and reporting. This build isn't particularly useful for any present application.

At present, only the `Release` version of the projects will build, but we're working on cleaning up the remainder of the configurations. As a temporary solution to accessing the features of the optimized version, you can set the constant in `production.h` to `0` regardless of the configuration. 

To build the client, find the `SwgClient` project in solution explorer and right click then select `Build`. Note that other projects may have similar names (like `ClientGame`) but these are shared across multiple tools. The actual game client you need for playing the game is the `SwgClient` project.

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
