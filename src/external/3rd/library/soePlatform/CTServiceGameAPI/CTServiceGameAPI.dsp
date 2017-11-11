# Microsoft Developer Studio Project File - Name="CTServiceGameAPI" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=CTServiceGameAPI - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CTServiceGameAPI.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CTServiceGameAPI.mak" CFG="CTServiceGameAPI - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CTServiceGameAPI - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "CTServiceGameAPI - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CTServiceGameAPI - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /I "../../../utils" /I "../" /I "./" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "EXTERNAL_DISTRO" /D NAMESPACE="CTService" /D "USE_TCP_LIBRARY" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "CTServiceGameAPI - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "." /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "EXTERNAL_DISTRO" /D NAMESPACE="CTService" /D "USE_TCP_LIBRARY" /YX /FD /GZ /c
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

# Name "CTServiceGameAPI - Win32 Release"
# Name "CTServiceGameAPI - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "CTGenericAPI Source Files"

# PROP Default_Filter ".cpp .c"
# Begin Source File

SOURCE=.\CTGenericAPI\GenericApiCore.cpp
# End Source File
# Begin Source File

SOURCE=.\CTGenericAPI\GenericConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\CTGenericAPI\GenericMessage.cpp
# End Source File
# End Group
# Begin Group "Base Source Files"

# PROP Default_Filter ".cpp c"
# Begin Source File

SOURCE=.\Base\Archive.cpp
# End Source File
# Begin Source File

SOURCE=.\Base\win32\Platform.cpp
# End Source File
# End Group
# Begin Group "TcpLibrary Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\TcpLibrary\Clock.cpp
# End Source File
# Begin Source File

SOURCE=.\TcpLibrary\IPAddress.cpp
# End Source File
# Begin Source File

SOURCE=.\TcpLibrary\TcpBlockAllocator.cpp
# End Source File
# Begin Source File

SOURCE=.\TcpLibrary\TcpConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\TcpLibrary\TcpManager.cpp
# End Source File
# End Group
# Begin Group "Unicode Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Unicode\FirstUnicode.cpp
# End Source File
# Begin Source File

SOURCE=.\Unicode\Unicode.cpp
# End Source File
# Begin Source File

SOURCE=.\Unicode\UnicodeBlocks.cpp
# End Source File
# Begin Source File

SOURCE=.\Unicode\UnicodeCharacterData.cpp
# End Source File
# Begin Source File

SOURCE=.\Unicode\UnicodeCharacterDataMap.cpp
# End Source File
# Begin Source File

SOURCE=.\Unicode\UnicodeCharTraits.cpp
# End Source File
# Begin Source File

SOURCE=.\Unicode\UnicodeUtils.cpp
# End Source File
# End Group
# Begin Group "CTCommon Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CTCommon\CTServiceCharacter.cpp
# End Source File
# Begin Source File

SOURCE=.\CTCommon\CTServiceServer.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\CTServiceAPI.cpp
# End Source File
# Begin Source File

SOURCE=.\CTServiceAPICore.cpp
# End Source File
# Begin Source File

SOURCE=.\Request.cpp
# End Source File
# Begin Source File

SOURCE=.\Response.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "CTGenericAPI Header Files"

# PROP Default_Filter ".h .hpp"
# Begin Source File

SOURCE=.\CTGenericAPI\GenericApiCore.h
# End Source File
# Begin Source File

SOURCE=.\CTGenericAPI\GenericConnection.h
# End Source File
# Begin Source File

SOURCE=.\CTGenericAPI\GenericMessage.h
# End Source File
# End Group
# Begin Group "Base Header Files"

# PROP Default_Filter ""
# Begin Group "win32 headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Base\win32\Archive.h
# End Source File
# Begin Source File

SOURCE=.\Base\win32\Platform.h
# End Source File
# Begin Source File

SOURCE=.\Base\win32\Types.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Base\Archive.h
# End Source File
# Begin Source File

SOURCE=.\Base\Platform.h
# End Source File
# Begin Source File

SOURCE=.\Base\Types.h
# End Source File
# End Group
# Begin Group "TcpLibrary Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\TcpLibrary\Clock.h
# End Source File
# Begin Source File

SOURCE=.\TcpLibrary\IPAddress.h
# End Source File
# Begin Source File

SOURCE=.\TcpLibrary\TcpBlockAllocator.h
# End Source File
# Begin Source File

SOURCE=.\TcpLibrary\TcpConnection.h
# End Source File
# Begin Source File

SOURCE=.\TcpLibrary\TcpHandlers.h
# End Source File
# Begin Source File

SOURCE=.\TcpLibrary\TcpManager.h
# End Source File
# End Group
# Begin Group "Unicode Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Unicode\FirstUnicode.h
# End Source File
# Begin Source File

SOURCE=.\Unicode\Unicode.h
# End Source File
# Begin Source File

SOURCE=.\Unicode\UnicodeBlocks.h
# End Source File
# Begin Source File

SOURCE=.\Unicode\UnicodeCharacterData.h
# End Source File
# Begin Source File

SOURCE=.\Unicode\UnicodeCharacterDataMap.h
# End Source File
# Begin Source File

SOURCE=.\Unicode\UnicodeUtils.h
# End Source File
# End Group
# Begin Group "CTCommon Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CTCommon\CTEnum.h
# End Source File
# Begin Source File

SOURCE=.\CTCommon\CTServiceCharacter.h
# End Source File
# Begin Source File

SOURCE=.\CTCommon\CTServiceObjects.h
# End Source File
# Begin Source File

SOURCE=.\CTCommon\CTServiceServer.h
# End Source File
# Begin Source File

SOURCE=.\CTCommon\RequestStrings.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\CTServiceAPI.h
# End Source File
# Begin Source File

SOURCE=.\CTServiceAPICore.h
# End Source File
# Begin Source File

SOURCE=.\Request.h
# End Source File
# Begin Source File

SOURCE=.\Response.h
# End Source File
# End Group
# End Target
# End Project
