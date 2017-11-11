# Microsoft Developer Studio Project File - Name="ChatAPI" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ChatAPI - Win32 Debug Threaded
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ChatAPI.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ChatAPI.mak" CFG="ChatAPI - Win32 Debug Threaded"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ChatAPI - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ChatAPI - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "ChatAPI - Win32 Release Threaded" (based on "Win32 (x86) Static Library")
!MESSAGE "ChatAPI - Win32 Debug Threaded" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ChatAPI - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../lib/release"
# PROP Intermediate_Dir "release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\..\utils" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ChatAPI - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../lib/debug"
# PROP Intermediate_Dir "debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\..\utils" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ChatAPI - Win32 Release Threaded"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ChatAPI___Win32_Release_Threaded"
# PROP BASE Intermediate_Dir "ChatAPI___Win32_Release_Threaded"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../lib/release"
# PROP Intermediate_Dir "release_mt"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\utils" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\lib\Release\ChatAPI.lib"
# ADD LIB32 /nologo /out:"../../../lib/release\ChatAPI_MT.lib"

!ELSEIF  "$(CFG)" == "ChatAPI - Win32 Debug Threaded"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ChatAPI___Win32_Debug_Threaded"
# PROP BASE Intermediate_Dir "ChatAPI___Win32_Debug_Threaded"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../lib/debug"
# PROP Intermediate_Dir "debug_mt"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\..\utils" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\..\utils" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\lib\Debug\ChatAPI.lib"
# ADD LIB32 /nologo /out:"../../../lib/debug\ChatAPI_MT.lib"

!ENDIF 

# Begin Target

# Name "ChatAPI - Win32 Release"
# Name "ChatAPI - Win32 Debug"
# Name "ChatAPI - Win32 Release Threaded"
# Name "ChatAPI - Win32 Debug Threaded"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AvatarIteratorCore.cpp
# End Source File
# Begin Source File

SOURCE=.\ChatAPI.cpp
# End Source File
# Begin Source File

SOURCE=.\ChatAPICore.cpp
# End Source File
# Begin Source File

SOURCE=.\ChatAvatar.cpp
# End Source File
# Begin Source File

SOURCE=.\ChatAvatarCore.cpp
# End Source File
# Begin Source File

SOURCE=.\ChatEnum.cpp
# End Source File
# Begin Source File

SOURCE=.\ChatFriendStatusCore.cpp
# End Source File
# Begin Source File

SOURCE=.\ChatIgnoreStatusCore.cpp
# End Source File
# Begin Source File

SOURCE=.\ChatRoom.cpp
# End Source File
# Begin Source File

SOURCE=.\ChatRoomCore.cpp
# End Source File
# Begin Source File

SOURCE=.\Message.cpp
# End Source File
# Begin Source File

SOURCE=.\PersistentMessage.cpp
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
# Begin Source File

SOURCE=.\AvatarIteratorCore.h
# End Source File
# Begin Source File

SOURCE=.\ChatAPI.h
# End Source File
# Begin Source File

SOURCE=.\ChatAPICore.h
# End Source File
# Begin Source File

SOURCE=.\ChatAvatar.h
# End Source File
# Begin Source File

SOURCE=.\ChatAvatarCore.h
# End Source File
# Begin Source File

SOURCE=.\ChatEnum.h
# End Source File
# Begin Source File

SOURCE=.\ChatFriendStatus.h
# End Source File
# Begin Source File

SOURCE=.\ChatFriendStatusCore.h
# End Source File
# Begin Source File

SOURCE=.\ChatIgnoreStatus.h
# End Source File
# Begin Source File

SOURCE=.\ChatIgnoreStatusCore.h
# End Source File
# Begin Source File

SOURCE=.\ChatRoom.h
# End Source File
# Begin Source File

SOURCE=.\ChatRoomCore.h
# End Source File
# Begin Source File

SOURCE=.\Message.h
# End Source File
# Begin Source File

SOURCE=.\PersistentMessage.h
# End Source File
# Begin Source File

SOURCE=.\PersistentMessageCore.h
# End Source File
# Begin Source File

SOURCE=.\Request.h
# End Source File
# Begin Source File

SOURCE=.\Response.h
# End Source File
# Begin Source File

SOURCE=.\RoomParamsCore.h
# End Source File
# Begin Source File

SOURCE=.\RoomSummaryCore.h
# End Source File
# End Group
# End Target
# End Project
