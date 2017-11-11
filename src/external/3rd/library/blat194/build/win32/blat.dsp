# Microsoft Developer Studio Project File - Name="blat" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=blat - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "blat.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "blat.mak" CFG="blat - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "blat - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "blat - Win32 Optimized" (based on "Win32 (x86) Static Library")
!MESSAGE "blat - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "blat"
# PROP Scc_LocalPath "..\.."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "blat - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\..\..\compile\win32\blat\Release"
# PROP Intermediate_Dir "..\..\..\..\..\..\compile\win32\blat\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /MT /W4 /WX /GR /GX /Zi /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "DEBUG_LEVEL=0" /D "_LIB" /D "GENSOCK_STATIC_LINK" /Yu"FirstBlat.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "blat - Win32 Optimized"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Optimized"
# PROP BASE Intermediate_Dir "Optimized"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\..\..\compile\win32\blat\Optimized"
# PROP Intermediate_Dir "..\..\..\..\..\..\compile\win32\blat\Optimized"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /MTd /W4 /WX /Gm /GR /GX /Zi /Ox /Ot /Og /Oi /Oy- /Gf /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "DEBUG_LEVEL=1" /D "_LIB" /D "GENSOCK_STATIC_LINK" /Yu"FirstBlat.h" /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "blat - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\..\..\compile\win32\blat\Debug"
# PROP Intermediate_Dir "..\..\..\..\..\..\compile\win32\blat\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MTd /W4 /WX /Gm /GR /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "DEBUG_LEVEL=2" /D "_LIB" /D "GENSOCK_STATIC_LINK" /Yu"FirstBlat.h" /FD /GZ /c
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

# Name "blat - Win32 Release"
# Name "blat - Win32 Optimized"
# Name "blat - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;rc"
# Begin Source File

SOURCE=..\..\src\win32\blat.cpp
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\blatcgi.cpp
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\FirstBlat.cpp
# ADD CPP /W3 /Yc
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\GENSOCK\gensock.cpp
# ADD CPP /W3
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "def;h;hpp;inl"
# Begin Source File

SOURCE=..\..\src\win32\blat.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\FirstBlat.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\GENSOCK\gensock.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Template Files"

# PROP Default_Filter "template"
# End Group
# End Target
# End Project
