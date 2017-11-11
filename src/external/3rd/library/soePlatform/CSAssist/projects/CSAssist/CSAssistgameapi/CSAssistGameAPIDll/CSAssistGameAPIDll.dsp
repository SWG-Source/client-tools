# Microsoft Developer Studio Project File - Name="CSAssistGameAPIDll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=CSAssistGameAPIDll - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CSAssistGameAPIDll.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CSAssistGameAPIDll.mak" CFG="CSAssistGameAPIDll - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CSAssistGameAPIDll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "CSAssistGameAPIDll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CSAssistGameAPIDll - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../libdll/release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CSASSISTGAMEAPIDLL_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../../../../utils" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "BUILD_CSADLL" /D "EXTERNAL_DISTRO" /D NAMESPACE="CSAssist" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib ws2_32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"../../libdll/release/CSAssistGameAPI.dll"

!ELSEIF  "$(CFG)" == "CSAssistGameAPIDll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../libdll/debug/"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CSASSISTGAMEAPIDLL_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../../../../utils" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "BUILD_CSADLL" /D "EXTERNAL_DISTRO" /D NAMESPACE="CSAssist" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib ws2_32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:"Debug/CSAssistGameAPI.pdb" /debug /machine:I386 /out:"../../libdll/debug/CSAssistGameAPI.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "CSAssistGameAPIDll - Win32 Release"
# Name "CSAssistGameAPIDll - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "CSAssistAPI Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\CSAssistgameapi.cpp
# End Source File
# Begin Source File

SOURCE=..\CSAssistgameapicore.cpp
# End Source File
# Begin Source File

SOURCE=..\CSAssistgameobjects.cpp
# End Source File
# Begin Source File

SOURCE=..\CSAssistreceiver.cpp
# End Source File
# Begin Source File

SOURCE=..\packdata.cpp
# End Source File
# Begin Source File

SOURCE=..\request.cpp
# End Source File
# Begin Source File

SOURCE=..\response.cpp
# End Source File
# End Group
# Begin Group "Base Source"

# PROP Default_Filter ""
# Begin Group "win32 source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\utils\Base\win32\BlockAllocator.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\..\utils\Base\Archive.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\utils\Base\Base.cpp
# End Source File
# End Group
# Begin Group "Unicode Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\utils\Unicode\FirstUnicode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\utils\Unicode\Unicode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\utils\Unicode\UnicodeBlocks.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\utils\Unicode\UnicodeCharacterData.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\utils\Unicode\UnicodeCharacterDataMap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\utils\Unicode\UnicodeCharTraits.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\utils\Unicode\UnicodeUtils.cpp
# End Source File
# End Group
# Begin Group "TcpLibrary Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\utils\TcpLibrary\Clock.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\utils\TcpLibrary\IPAddress.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\utils\TcpLibrary\TcpBlockAllocator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\utils\TcpLibrary\TcpConnection.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\utils\TcpLibrary\TcpManager.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\CSAssistGameAPIDll.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Base Header Files"

# PROP Default_Filter ""
# Begin Group "win32 headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\utils\Base\win32\Archive.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\..\utils\Base\Archive.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\utils\Base\Base.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\utils\Base\BlockAllocator.h
# End Source File
# End Group
# Begin Group "Unicode Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\utils\Unicode\FirstUnicode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\utils\Unicode\Unicode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\utils\Unicode\UnicodeBlocks.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\utils\Unicode\UnicodeCharacterData.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\utils\Unicode\UnicodeCharacterDataMap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\utils\Unicode\UnicodeUtils.h
# End Source File
# End Group
# Begin Group "TcpLibrary Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\utils\TcpLibrary\Clock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\utils\TcpLibrary\IPAddress.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\utils\TcpLibrary\TcpBlockAllocator.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\utils\TcpLibrary\TcpConnection.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\utils\TcpLibrary\TcpHandlers.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\utils\TcpLibrary\TcpManager.h
# End Source File
# End Group
# Begin Group "CSAssist Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\CSAssistgameapi.h
# End Source File
# Begin Source File

SOURCE=..\CSAssistgameapicore.h
# End Source File
# Begin Source File

SOURCE=..\CSAssistgameobjects.h
# End Source File
# Begin Source File

SOURCE=..\CSAssistreceiver.h
# End Source File
# Begin Source File

SOURCE=..\packdata.h
# End Source File
# Begin Source File

SOURCE=..\request.h
# End Source File
# Begin Source File

SOURCE=..\response.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\CSAssistGameAPIDll.h
# End Source File
# End Group
# End Target
# End Project
