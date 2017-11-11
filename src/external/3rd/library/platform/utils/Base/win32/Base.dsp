# Microsoft Developer Studio Project File - Name="Base" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Base - Win32 Debug Threaded
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Base.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Base.mak" CFG="Base - Win32 Debug Threaded"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Base - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Base - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Base - Win32 Debug Threaded" (based on "Win32 (x86) Static Library")
!MESSAGE "Base - Win32 Release Threaded" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Base - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../lib/release"
# PROP Intermediate_Dir "../release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../.." /I "../" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Base - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../lib/debug"
# PROP Intermediate_Dir "../debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../.." /I "../" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Base - Win32 Debug Threaded"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Base___Win32_Debug_Threaded"
# PROP BASE Intermediate_Dir "Base___Win32_Debug_Threaded"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../lib/debug"
# PROP Intermediate_Dir "../debug_mt"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../.." /I "../" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../.." /I "../" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../../../lib/debug\Base_MT.lib"

!ELSEIF  "$(CFG)" == "Base - Win32 Release Threaded"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Base___Win32_Release_Threaded"
# PROP BASE Intermediate_Dir "Base___Win32_Release_Threaded"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../lib/release"
# PROP Intermediate_Dir "../release_mt"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "../.." /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../.." /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../../../lib/release\Base_MT.lib"

!ENDIF 

# Begin Target

# Name "Base - Win32 Release"
# Name "Base - Win32 Debug"
# Name "Base - Win32 Debug Threaded"
# Name "Base - Win32 Release Threaded"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "win32 source"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=.\BlockAllocator.cpp
# End Source File
# Begin Source File

SOURCE=.\Event.cpp
# End Source File
# Begin Source File

SOURCE=.\Logger.cpp
# End Source File
# Begin Source File

SOURCE=.\Mutex.cpp
# End Source File
# Begin Source File

SOURCE=.\Platform.cpp
# End Source File
# Begin Source File

SOURCE=.\Thread.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\Archive.cpp
# End Source File
# Begin Source File

SOURCE=..\AutoLog.cpp
# End Source File
# Begin Source File

SOURCE=..\Base.cpp
# End Source File
# Begin Source File

SOURCE=..\Config.cpp
# End Source File
# Begin Source File

SOURCE=..\MD5.cpp
# End Source File
# Begin Source File

SOURCE=..\ScopeLock.cpp
# End Source File
# Begin Source File

SOURCE=..\Statistics.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "win32 headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\Archive.h
# End Source File
# Begin Source File

SOURCE=.\Event.h
# End Source File
# Begin Source File

SOURCE=.\Mutex.h
# End Source File
# Begin Source File

SOURCE=.\Platform.h
# End Source File
# Begin Source File

SOURCE=.\Thread.h
# End Source File
# Begin Source File

SOURCE=.\Types.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\Archive.h
# End Source File
# Begin Source File

SOURCE=..\AutoLog.h
# End Source File
# Begin Source File

SOURCE=..\Base.h
# End Source File
# Begin Source File

SOURCE=..\BlockAllocator.h
# End Source File
# Begin Source File

SOURCE=..\Config.h
# End Source File
# Begin Source File

SOURCE=..\Event.h
# End Source File
# Begin Source File

SOURCE=..\Logger.h
# End Source File
# Begin Source File

SOURCE=..\MD5.h
# End Source File
# Begin Source File

SOURCE=..\Mutex.h
# End Source File
# Begin Source File

SOURCE=..\Platform.h
# End Source File
# Begin Source File

SOURCE=..\ScopeLock.h
# End Source File
# Begin Source File

SOURCE=..\Statistics.h
# End Source File
# Begin Source File

SOURCE=..\TemplateBlockAllocator.h
# End Source File
# Begin Source File

SOURCE=..\TemplateObjectAllocator.h
# End Source File
# Begin Source File

SOURCE=..\Thread.h
# End Source File
# Begin Source File

SOURCE=..\Types.h
# End Source File
# End Group
# End Target
# End Project
