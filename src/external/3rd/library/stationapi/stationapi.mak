# Microsoft Developer Studio Generated NMAKE File, Based on stationapi.dsp
!IF "$(CFG)" == ""
CFG=stationapi - Win32 Debug
!MESSAGE No configuration specified. Defaulting to stationapi - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "stationapi - Win32 Release" && "$(CFG)" != "stationapi - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "stationapi.mak" CFG="stationapi - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "stationapi - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "stationapi - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "stationapi - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\stationapi.lib"


CLEAN :
	-@erase "$(INTDIR)\extend_rdp.obj"
	-@erase "$(INTDIR)\order.obj"
	-@erase "$(INTDIR)\PackClass.obj"
	-@erase "$(INTDIR)\stationapi.obj"
	-@erase "$(INTDIR)\stationapilist.obj"
	-@erase "$(INTDIR)\StationAPISession.obj"
	-@erase "$(INTDIR)\stationrequest.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\stationapi.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "c:/rdp/include" /I "c:/rdp/rdp" /I "c:/rdp/dbgutil" /I "../crypt" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Fp"$(INTDIR)\stationapi.pch" /YX"stationapi.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\stationapi.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\stationapi.lib" 
LIB32_OBJS= \
	"$(INTDIR)\extend_rdp.obj" \
	"$(INTDIR)\PackClass.obj" \
	"$(INTDIR)\stationapi.obj" \
	"$(INTDIR)\stationapilist.obj" \
	"$(INTDIR)\StationAPISession.obj" \
	"$(INTDIR)\stationrequest.obj" \
	"$(INTDIR)\order.obj"

"$(OUTDIR)\stationapi.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "stationapi - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\stationapi.lib"


CLEAN :
	-@erase "$(INTDIR)\extend_rdp.obj"
	-@erase "$(INTDIR)\order.obj"
	-@erase "$(INTDIR)\PackClass.obj"
	-@erase "$(INTDIR)\stationapi.obj"
	-@erase "$(INTDIR)\stationapilist.obj"
	-@erase "$(INTDIR)\StationAPISession.obj"
	-@erase "$(INTDIR)\stationrequest.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\stationapi.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "c:/rdp/include" /I "c:/rdp/rdp" /I "c:/rdp/dbgutil" /I "../crypt" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Fp"$(INTDIR)\stationapi.pch" /YX"stationapi.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\stationapi.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\stationapi.lib" 
LIB32_OBJS= \
	"$(INTDIR)\extend_rdp.obj" \
	"$(INTDIR)\PackClass.obj" \
	"$(INTDIR)\stationapi.obj" \
	"$(INTDIR)\stationapilist.obj" \
	"$(INTDIR)\StationAPISession.obj" \
	"$(INTDIR)\stationrequest.obj" \
	"$(INTDIR)\order.obj"

"$(OUTDIR)\stationapi.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("stationapi.dep")
!INCLUDE "stationapi.dep"
!ELSE 
!MESSAGE Warning: cannot find "stationapi.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "stationapi - Win32 Release" || "$(CFG)" == "stationapi - Win32 Debug"
SOURCE=.\extend_rdp.c

"$(INTDIR)\extend_rdp.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\order.cpp

"$(INTDIR)\order.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\PackClass.cpp

"$(INTDIR)\PackClass.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\stationapi.cpp

"$(INTDIR)\stationapi.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\stationapilist.cpp

"$(INTDIR)\stationapilist.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\StationAPISession.cpp

"$(INTDIR)\StationAPISession.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\stationrequest.cpp

"$(INTDIR)\stationrequest.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

