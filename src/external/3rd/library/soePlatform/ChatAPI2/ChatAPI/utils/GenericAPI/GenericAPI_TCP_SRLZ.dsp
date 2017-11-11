# Microsoft Developer Studio Project File - Name="GenericAPI_TCP_SRLZ" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=GenericAPI_TCP_SRLZ - Win32 Debug Threaded
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GenericAPI_TCP_SRLZ.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GenericAPI_TCP_SRLZ.mak" CFG="GenericAPI_TCP_SRLZ - Win32 Debug Threaded"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GenericAPI_TCP_SRLZ - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "GenericAPI_TCP_SRLZ - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "GenericAPI_TCP_SRLZ - Win32 Debug Threaded" (based on "Win32 (x86) Static Library")
!MESSAGE "GenericAPI_TCP_SRLZ - Win32 Release Threaded" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GenericAPI_TCP_SRLZ - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../lib/release"
# PROP Intermediate_Dir "release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USE_TCP_LIBRARY" /D "USE_SERIALIZE_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "GenericAPI_TCP_SRLZ - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "GenericAPI_TCP_SRLZ___Win32_Debug"
# PROP BASE Intermediate_Dir "GenericAPI_TCP_SRLZ___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../lib/debug"
# PROP Intermediate_Dir "debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USE_TCP_LIBRARY" /D "USE_SERIALIZE_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "GenericAPI_TCP_SRLZ - Win32 Debug Threaded"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug Threaded"
# PROP BASE Intermediate_Dir "Debug Threaded"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../lib/debug"
# PROP Intermediate_Dir "debug_mt"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USE_TCP_LIBRARY" /D "USE_SERIALIZE_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../../lib/debug\GenericAPI_TCP_SRLZ_MT.lib"

!ELSEIF  "$(CFG)" == "GenericAPI_TCP_SRLZ - Win32 Release Threaded"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release Threaded"
# PROP BASE Intermediate_Dir "Release Threaded"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../lib/release"
# PROP Intermediate_Dir "release_mt"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USE_TCP_LIBRARY" /D "USE_SERIALIZE_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../../lib/release\GenericAPI_TCP_SRLZ_MT.lib"

!ENDIF 

# Begin Target

# Name "GenericAPI_TCP_SRLZ - Win32 Release"
# Name "GenericAPI_TCP_SRLZ - Win32 Debug"
# Name "GenericAPI_TCP_SRLZ - Win32 Debug Threaded"
# Name "GenericAPI_TCP_SRLZ - Win32 Release Threaded"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\GenericApiCore.cpp
# End Source File
# Begin Source File

SOURCE=.\GenericConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\GenericMessage.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\GenericApiCore.h
# End Source File
# Begin Source File

SOURCE=.\GenericConnection.h
# End Source File
# Begin Source File

SOURCE=.\GenericMessage.h
# End Source File
# End Group
# End Target
# End Project
