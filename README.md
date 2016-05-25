# Client/Tools Repo

This repo is only for building the clients and tools. 

## Branches:

* **master** - currently builds using Visual Studio 2013. Uses the old stlport.
* **vs2015**-stlconversion - Current "work in progress" for building on Visual Studio 2015, which includes it's own, complete STL implementation.
* **msvc-stl-conversion** - old attempt at using VS2013 STL. May or may not be useful for reference, but likely not useful for building.

## Visual Studio

Visual Studio Community for most, if not all versions, is free for 90 days, after which you may need to do something slightly illegal, unless you have DreamSpark: https://www.visualstudio.com/en-us/downloads/download-visual-studio-vs.aspx

## Building

To build a client, find and right click "SwgClient" after choosing the configuration you wish to use (Debug, Optimized, Release). Click build.

The same goes for the other projects but they are hit and miss as far as being working or not. If you hit a bug, please fix and pull request! It is suggested you ignore the "server" projects and server only libs as we have no use for those.

## Known Issues

* For the debug build, and possibly the optimized versions, you will get linker errors about libmozilla - if you alter the project settings you can disable this from killing the output of an exe, as libmozilla is only needed for the ingame browser.

* Other linker errors sometimes throw, you have to work on these case by case. Please pull request any changes you make.

* cmd.exe issues sometimes occur as SOE originally had the build setup copying files to a proper game bin directory. You can just remove these from projects that complain about them, just copy the output files manually.

* Plenty of warnings and sometimes even errors regarding deprecated libs happen. Fixes for these are case by case.
