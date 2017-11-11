# Microsoft Developer Studio Project File - Name="CSAssistgameapi" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=CSAssistgameapi - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CSAssistgameapi.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CSAssistgameapi.mak" CFG="CSAssistgameapi - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CSAssistgameapi - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "CSAssistgameapi - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CSAssistgameapi - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../../../utils" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "EXTERNAL_DISTRO" /D NAMESPACE="CSAssist" /D "USE_TCP_LIBRARY" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\release\CSAssistgameapi.lib"

!ELSEIF  "$(CFG)" == "CSAssistgameapi - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\lib\debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../../../utils" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "EXTERNAL_DISTRO" /D NAMESPACE="CSAssist" /FR /YX /FD /GZ /c
# SUBTRACT CPP /WX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "CSAssistgameapi - Win32 Release"
# Name "CSAssistgameapi - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "CSAssistAPI Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CSAssistgameapi.cpp
# End Source File
# Begin Source File

SOURCE=.\CSAssistgameapicore.cpp
# End Source File
# Begin Source File

SOURCE=.\CSAssistgameobjects.cpp
# End Source File
# Begin Source File

SOURCE=.\CSAssistreceiver.cpp
# End Source File
# Begin Source File

SOURCE=.\packdata.cpp
# End Source File
# Begin Source File

SOURCE=.\request.cpp
# End Source File
# Begin Source File

SOURCE=.\response.cpp
# End Source File
# End Group
# Begin Group "Base Source"

# PROP Default_Filter ""
# Begin Group "win32 source"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=..\..\..\utils\Base\win32\BlockAllocator.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\utils\Base\Archive.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\Base.cpp
# End Source File
# End Group
# Begin Group "Unicode Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\utils\Unicode\FirstUnicode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Unicode\Unicode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Unicode\UnicodeBlocks.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Unicode\UnicodeCharacterData.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Unicode\UnicodeCharacterDataMap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Unicode\UnicodeCharTraits.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Unicode\UnicodeUtils.cpp
# End Source File
# End Group
# Begin Group "TcpLibrary Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\utils\TcpLibrary\Clock.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\TcpLibrary\IPAddress.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\TcpLibrary\TcpBlockAllocator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\TcpLibrary\TcpConnection.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\TcpLibrary\TcpManager.cpp
# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Base Header Files"

# PROP Default_Filter ""
# Begin Group "win32 headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=..\..\..\utils\Base\win32\Archive.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\utils\Base\Archive.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\Base.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\BlockAllocator.h
# End Source File
# End Group
# Begin Group "Unicode Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\utils\Unicode\FirstUnicode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Unicode\Unicode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Unicode\UnicodeBlocks.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Unicode\UnicodeCharacterData.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Unicode\UnicodeCharacterDataMap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Unicode\UnicodeUtils.h
# End Source File
# End Group
# Begin Group "TcpLibrary Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\utils\TcpLibrary\Clock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\TcpLibrary\IPAddress.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\TcpLibrary\TcpBlockAllocator.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\TcpLibrary\TcpConnection.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\TcpLibrary\TcpHandlers.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\TcpLibrary\TcpManager.h
# End Source File
# End Group
# Begin Group "CSAssist Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CSAssistgameapi.h
# End Source File
# Begin Source File

SOURCE=.\CSAssistgameapicore.h
# End Source File
# Begin Source File

SOURCE=.\CSAssistgameobjects.h
# End Source File
# Begin Source File

SOURCE=.\CSAssistreceiver.h
# End Source File
# Begin Source File

SOURCE=.\packdata.h
# End Source File
# Begin Source File

SOURCE=.\request.h
# End Source File
# Begin Source File

SOURCE=.\response.h
# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=.\release.txt
# End Source File
# End Target
# End Project
