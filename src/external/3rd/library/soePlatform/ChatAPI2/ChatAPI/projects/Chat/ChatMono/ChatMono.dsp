# Microsoft Developer Studio Project File - Name="ChatMono" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ChatMono - Win32 Debug Threaded
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ChatMono.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ChatMono.mak" CFG="ChatMono - Win32 Debug Threaded"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ChatMono - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ChatMono - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "ChatMono - Win32 Debug Threaded" (based on "Win32 (x86) Static Library")
!MESSAGE "ChatMono - Win32 Release Threaded" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ChatMono - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\..\utils" /D "NDEBUG" /D NAMESPACE="ChatSystem" /D "WIN32" /D "_MBCS" /D "_LIB" /D "EXTERNAL_DISTRO" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ChatMono - Win32 Debug"

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
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\..\utils" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "EXTERNAL_DISTRO" /D NAMESPACE="ChatSystem" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ChatMono - Win32 Debug Threaded"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ChatMono___Win32_Debug_Threaded0"
# PROP BASE Intermediate_Dir "ChatMono___Win32_Debug_Threaded0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "ChatMono___Win32_Debug_Threaded0"
# PROP Intermediate_Dir "ChatMono___Win32_Debug_Threaded0"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\..\utils" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "EXTERNAL_DISTRO" /D NAMESPACE="ChatSystem" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\..\utils" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "EXTERNAL_DISTRO" /D NAMESPACE="ChatSystem" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ChatMono - Win32 Release Threaded"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ChatMono___Win32_Release_Threaded0"
# PROP BASE Intermediate_Dir "ChatMono___Win32_Release_Threaded0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ChatMono___Win32_Release_Threaded0"
# PROP Intermediate_Dir "ChatMono___Win32_Release_Threaded0"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\..\..\utils" /D "NDEBUG" /D NAMESPACE="ChatSystem" /D "WIN32" /D "_MBCS" /D "_LIB" /D "EXTERNAL_DISTRO" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\utils" /D "NDEBUG" /D NAMESPACE="ChatSystem" /D "WIN32" /D "_MBCS" /D "_LIB" /D "EXTERNAL_DISTRO" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "ChatMono - Win32 Release"
# Name "ChatMono - Win32 Debug"
# Name "ChatMono - Win32 Debug Threaded"
# Name "ChatMono - Win32 Release Threaded"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Chat Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\ChatAPI\AvatarIteratorCore.cpp
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\ChatAPI.cpp
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\ChatAPICore.cpp
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\ChatAvatar.cpp
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\ChatAvatarCore.cpp
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\ChatEnum.cpp
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\ChatFriendStatusCore.cpp
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\ChatIgnoreStatusCore.cpp
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\ChatRoom.cpp
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\ChatRoomCore.cpp
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\Message.cpp
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\PersistentMessage.cpp
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\Request.cpp
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\Response.cpp
# End Source File
# End Group
# Begin Group "Base Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\utils\Base\Archive.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\AutoLog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\Base.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\win32\BlockAllocator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\Config.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\win32\Event.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\MD5.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\win32\Mutex.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\win32\Platform.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\ScopeLock.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\Statistics.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\win32\Thread.cpp
# End Source File
# End Group
# Begin Group "UdpLibrary Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\utils\UdpLibrary\UdpLibrary.cpp
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
# Begin Group "GenericAPI Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\utils\GenericAPI\GenericApiCore.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\GenericAPI\GenericConnection.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\GenericAPI\GenericMessage.cpp
# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Chat Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\ChatAPI\AvatarIteratorCore.h
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\ChatAPI.h
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\ChatAPICore.h
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\ChatAvatar.h
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\ChatAvatarCore.h
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\ChatEnum.h
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\ChatFriendStatus.h
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\ChatFriendStatusCore.h
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\ChatIgnoreStatus.h
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\ChatIgnoreStatusCore.h
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\ChatRoom.h
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\ChatRoomCore.h
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\Message.h
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\PersistentMessage.h
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\PersistentMessageCore.h
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\Request.h
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\Response.h
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\RoomParamsCore.h
# End Source File
# Begin Source File

SOURCE=..\ChatAPI\RoomSummaryCore.h
# End Source File
# End Group
# Begin Group "Base Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\utils\Base\Archive.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\win32\Archive.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\AutoLog.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\Base.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\BlockAllocator.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\Config.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\Event.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\win32\Event.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\MD5.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\Mutex.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\win32\Mutex.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\Platform.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\win32\Platform.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\ScopeLock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\Statistics.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\TemplateBlockAllocator.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\TemplateObjectAllocator.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\Thread.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\win32\Thread.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\Types.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\Base\win32\Types.h
# End Source File
# End Group
# Begin Group "UdpLibrary Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\utils\UdpLibrary\hashtable.hpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\UdpLibrary\PointerDeque.hpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\UdpLibrary\priority.hpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\UdpLibrary\UdpHandler.hpp
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\UdpLibrary\UdpLibrary.hpp
# End Source File
# End Group
# Begin Group "Unicode Header"

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
# Begin Group "GenericAPI Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\utils\GenericAPI\GenericApiCore.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\GenericAPI\GenericConnection.h
# End Source File
# Begin Source File

SOURCE=..\..\..\utils\GenericAPI\GenericMessage.h
# End Source File
# End Group
# End Group
# End Target
# End Project
