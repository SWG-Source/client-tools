# Microsoft Developer Studio Project File - Name="UIFontBuilder" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=UIFontBuilder - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "UIFontBuilder.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "UIFontBuilder.mak" CFG="UIFontBuilder - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "UIFontBuilder - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "UIFontBuilder - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "UIFontBuilder"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "UIFontBuilder - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../../compile/win32/UIFontBuilder/Release"
# PROP Intermediate_Dir "../../../../compile/win32/UIFontBuilder/Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /G6 /MT /W4 /WX /GR /GX /O2 /I "../../../../external/ours/library/unicode/include" /I "../../../../external/3rd/library/directx9/include" /I "../../../../external/3rd/library/stlport453/stlport" /I "../../../../external/3rd/library/ui/include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D _WIN32_WINNT=0x0501 /Yu"stdafx.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib shlwapi.lib comctl32.lib winmm.lib ddraw.lib wsock32.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /machine:I386 /include:"__imp__ExtractIconW@12" /libpath:"../../library/directx9/lib" /libpath:"../../library/stlport453/lib/win32"

!ELSEIF  "$(CFG)" == "UIFontBuilder - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../compile/win32/UIFontBuilder/Debug"
# PROP Intermediate_Dir "../../../../compile/win32/UIFontBuilder/Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /G6 /MTd /W4 /WX /Gm /GR /GX /Zi /Od /I "../../../../external/ours/library/unicode/include" /I "../../../../external/3rd/library/directx9/include" /I "../../../../external/3rd/library/stlport453/stlport" /I "../../../../external/3rd/library/ui/include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D _WIN32_WINNT=0x0501 /Yu"stdafx.h" /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib shlwapi.lib comctl32.lib winmm.lib ddraw.lib wsock32.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcmt" /pdbtype:sept /libpath:"../../library/directx9/lib" /libpath:"../../library/stlport453/lib/win32"

!ENDIF 

# Begin Target

# Name "UIFontBuilder - Win32 Release"
# Name "UIFontBuilder - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\FontExporter.cpp
# End Source File
# Begin Source File

SOURCE=.\FontExporterPSUI.cpp
# End Source File
# Begin Source File

SOURCE=.\FontGlyphCode.cpp
# End Source File
# Begin Source File

SOURCE=.\FontImageExporter.cpp
# End Source File
# Begin Source File

SOURCE=.\FontImageExporterBMP.cpp
# End Source File
# Begin Source File

SOURCE=.\FontImageExporterDDS.cpp
# End Source File
# Begin Source File

SOURCE=.\PreviewDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\UIFontBuilder.cpp
# End Source File
# Begin Source File

SOURCE=.\UIFontBuilder.rc
# End Source File
# Begin Source File

SOURCE=.\UIFontBuilderDlg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\FontExporter.h
# End Source File
# Begin Source File

SOURCE=.\FontExporterPSUI.h
# End Source File
# Begin Source File

SOURCE=.\FontGlyphCode.h
# End Source File
# Begin Source File

SOURCE=.\FontImageExporter.h
# End Source File
# Begin Source File

SOURCE=.\FontImageExporterBMP.h
# End Source File
# Begin Source File

SOURCE=.\FontImageExporterDDS.h
# End Source File
# Begin Source File

SOURCE=.\PreviewDialog.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\UIFontBuilder.h
# End Source File
# Begin Source File

SOURCE=.\UIFontBuilderDlg.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap_t.bmp
# End Source File
# Begin Source File

SOURCE=.\res\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\res\cursor2.cur
# End Source File
# Begin Source File

SOURCE=.\res\UIFontBuilder.ico
# End Source File
# Begin Source File

SOURCE=.\res\UIFontBuilder.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
