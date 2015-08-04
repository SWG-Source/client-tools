# Microsoft Developer Studio Project File - Name="UIBuilder" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=UIBuilder - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "UIBuilder.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "UIBuilder.mak" CFG="UIBuilder - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "UIBuilder - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "UIBuilder - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/tools/UIBuilder", QKIAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "UIBuilder - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../../compile/win32/UIBuilder/Release"
# PROP Intermediate_Dir "../../../../compile/win32/UIBuilder/Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G6 /MT /W4 /WX /GR /GX /O2 /I "../../../../external/ours/library/unicode/include" /I "../../../../external/3rd/library/directx9/include" /I "../../../../external/3rd/library/stlport453/stlport" /I "../../../../external/3rd/library/ui/include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "SWG_HACK" /Yu"FirstUiBuilder.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 dxguid.lib d3dx.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib shlwapi.lib comctl32.lib winmm.lib ddraw.lib wsock32.lib dsound.lib d3d9.lib /nologo /subsystem:windows /debug /machine:I386 /libpath:"../../library/directx9/lib" /libpath:"../../library/stlport453/lib/win32"

!ELSEIF  "$(CFG)" == "UIBuilder - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../compile/win32/UIBuilder/Debug"
# PROP Intermediate_Dir "../../../../compile/win32/UIBuilder/Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MTd /W4 /WX /Gm /GR /GX /Zi /Od /I "../../../../external/ours/library/unicode/include" /I "../../../../external/3rd/library/directx9/include" /I "../../../../external/3rd/library/stlport453/stlport" /I "../../../../external/3rd/library/ui/include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "SWG_HACK" /Yu"FirstUiBuilder.h" /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 dxguid.lib d3dx.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib shlwapi.lib comctl32.lib winmm.lib ddraw.lib wsock32.lib dsound.lib d3d9.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcmt" /pdbtype:sept /libpath:"../../library/directx9/lib" /libpath:"../../library/stlport453/lib/win32"

!ENDIF 

# Begin Target

# Name "UIBuilder - Win32 Release"
# Name "UIBuilder - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AddPropertyDialogBox.cpp
# End Source File
# Begin Source File

SOURCE=.\DDSCanvasGenerator.cpp
# End Source File
# Begin Source File

SOURCE=.\DefaultObjectPropertiesManager.cpp
# End Source File
# Begin Source File

SOURCE=.\DiffDialogBox.cpp
# End Source File
# Begin Source File

SOURCE=.\EditPropertyDialogBox.cpp
# End Source File
# Begin Source File

SOURCE=.\FeatureLog.cpp
# End Source File
# Begin Source File

SOURCE=.\FileLocator.cpp
# End Source File
# Begin Source File

SOURCE=.\FirstUiBuilder.cpp
# ADD CPP /Yc
# End Source File
# Begin Source File

SOURCE=.\GridSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\ImportImage.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectInspector.cpp
# End Source File
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# Begin Source File

SOURCE=.\SelectionRect.cpp
# End Source File
# Begin Source File

SOURCE=.\SelectionSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\SelectRegion.cpp
# End Source File
# Begin Source File

SOURCE=.\SimpleSoundCanvas.cpp
# End Source File
# Begin Source File

SOURCE=.\UIBuilderHistory.cpp
# End Source File
# Begin Source File

SOURCE=.\UIDirect3DPrimaryCanvas.cpp
# End Source File
# Begin Source File

SOURCE=.\UIDirect3DTextureCanvas.cpp
# End Source File
# Begin Source File

SOURCE=.\UIDirectDrawCanvas.cpp
# End Source File
# Begin Source File

SOURCE=.\UIDirectDrawOffscreenCanvas.cpp
# End Source File
# Begin Source File

SOURCE=.\UIDirectDrawPrimaryCanvas.cpp
# End Source File
# Begin Source File

SOURCE=.\UIDirectSoundSoundCanvas.cpp
# End Source File
# Begin Source File

SOURCE=.\UIWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\WinMain.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AddPropertyDialogBox.h
# End Source File
# Begin Source File

SOURCE=.\DDSCanvasGenerator.h
# End Source File
# Begin Source File

SOURCE=.\DefaultObjectPropertiesManager.h
# End Source File
# Begin Source File

SOURCE=.\DiffDialogBox.h
# End Source File
# Begin Source File

SOURCE=.\EditPropertyDialogBox.h
# End Source File
# Begin Source File

SOURCE=.\FileLocator.h
# End Source File
# Begin Source File

SOURCE=.\FirstUiBuilder.h
# End Source File
# Begin Source File

SOURCE=.\ObjectInspector.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\SelectionRect.h
# End Source File
# Begin Source File

SOURCE=.\SelectRegion.h
# End Source File
# Begin Source File

SOURCE=.\SimpleSoundCanvas.h
# End Source File
# Begin Source File

SOURCE=.\UIBuilderHistory.h
# End Source File
# Begin Source File

SOURCE=.\UIDirect3DPrimaryCanvas.h
# End Source File
# Begin Source File

SOURCE=.\UIDirect3DTextureCanvas.h
# End Source File
# Begin Source File

SOURCE=.\UIDirectDrawCanvas.h
# End Source File
# Begin Source File

SOURCE=.\UIDirectDrawOffscreenCanvas.h
# End Source File
# Begin Source File

SOURCE=.\UIDirectDrawPrimaryCanvas.h
# End Source File
# Begin Source File

SOURCE=.\UIDirectSoundSoundCanvas.h
# End Source File
# Begin Source File

SOURCE=..\..\library\ui\src\shared\core\UIPaletteRegistrySetup.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\addprop.ico
# End Source File
# Begin Source File

SOURCE=.\addsel.cur
# End Source File
# Begin Source File

SOURCE=.\alignbot.ico
# End Source File
# Begin Source File

SOURCE=.\alignhc.ico
# End Source File
# Begin Source File

SOURCE=.\alignlft.ico
# End Source File
# Begin Source File

SOURCE=.\alignrt.ico
# End Source File
# Begin Source File

SOURCE=.\aligntop.ico
# End Source File
# Begin Source File

SOURCE=.\alignvc.ico
# End Source File
# Begin Source File

SOURCE=.\app.ico
# End Source File
# Begin Source File

SOURCE=.\bottom.ico
# End Source File
# Begin Source File

SOURCE=.\buttons.bmp
# End Source File
# Begin Source File

SOURCE=.\cursor.ico
# End Source File
# Begin Source File

SOURCE=.\down.ico
# End Source File
# Begin Source File

SOURCE=.\drawgrid.ico
# End Source File
# Begin Source File

SOURCE=.\drawsel.ico
# End Source File
# Begin Source File

SOURCE=.\folder.ico
# End Source File
# Begin Source File

SOURCE=.\lock.ico
# End Source File
# Begin Source File

SOURCE=.\lock1.ico
# End Source File
# Begin Source File

SOURCE=.\p4.bmp
# End Source File
# Begin Source File

SOURCE=.\play.ico
# End Source File
# Begin Source File

SOURCE=.\refresh.ico
# End Source File
# Begin Source File

SOURCE=.\remprop.ico
# End Source File
# Begin Source File

SOURCE=.\remsel.cur
# End Source File
# Begin Source File

SOURCE=.\reset.ico
# End Source File
# Begin Source File

SOURCE=.\sizehigh.ico
# End Source File
# Begin Source File

SOURCE=.\sizewide.ico
# End Source File
# Begin Source File

SOURCE=.\snapgrid.ico
# End Source File
# Begin Source File

SOURCE=.\tooltip.ico
# End Source File
# Begin Source File

SOURCE=.\top.ico
# End Source File
# Begin Source File

SOURCE=.\up.ico
# End Source File
# Begin Source File

SOURCE=.\viewimag.ico
# End Source File
# Begin Source File

SOURCE=.\zoomin.ico
# End Source File
# Begin Source File

SOURCE=.\zoomout.ico
# End Source File
# End Group
# End Target
# End Project
