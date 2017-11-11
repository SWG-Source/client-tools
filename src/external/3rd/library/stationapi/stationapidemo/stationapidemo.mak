# Microsoft Developer Studio Generated NMAKE File, Based on stationapidemo.dsp
!IF "$(CFG)" == ""
CFG=stationapidemo - Win32 Debug
!MESSAGE No configuration specified. Defaulting to stationapidemo - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "stationapidemo - Win32 Release" && "$(CFG)" != "stationapidemo - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "stationapidemo.mak" CFG="stationapidemo - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "stationapidemo - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "stationapidemo - Win32 Debug" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "stationapidemo - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\stationapidemo.exe"

!ELSE 

ALL : "989crypt - Win32 Release" "tsip - Win32 Release" "stationapi - Win32 Release" "$(OUTDIR)\stationapidemo.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"stationapi - Win32 ReleaseCLEAN" "tsip - Win32 ReleaseCLEAN" "989crypt - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\stationapidemo.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\stationapidemo.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "c:/rdp/include" /I "c:/rdp/rdp" /I "c:/rdp/dbgutil" /I ".." /I "../../crypt" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\stationapidemo.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\stationapidemo.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=rdp.lib 989crypt.lib wsock32.lib winmm.lib stationapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\stationapidemo.pdb" /machine:I386 /out:"$(OUTDIR)\stationapidemo.exe" /libpath:"..\release" /libpath:"c:\rdp\rdp" /libpath:"..\..\crypt\release" 
LINK32_OBJS= \
	"$(INTDIR)\stationapidemo.obj" \
	"..\Release\stationapi.lib" \
	"..\..\tsip\Release\tsip.lib" \
	"..\..\crypt\Release\989crypt.lib"

"$(OUTDIR)\stationapidemo.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "stationapidemo - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\stationapidemo.exe" "$(OUTDIR)\stationapidemo.bsc"

!ELSE 

ALL : "989crypt - Win32 Debug" "tsip - Win32 Debug" "stationapi - Win32 Debug" "$(OUTDIR)\stationapidemo.exe" "$(OUTDIR)\stationapidemo.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"stationapi - Win32 DebugCLEAN" "tsip - Win32 DebugCLEAN" "989crypt - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\stationapidemo.obj"
	-@erase "$(INTDIR)\stationapidemo.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\stationapidemo.bsc"
	-@erase "$(OUTDIR)\stationapidemo.exe"
	-@erase "$(OUTDIR)\stationapidemo.ilk"
	-@erase "$(OUTDIR)\stationapidemo.map"
	-@erase "$(OUTDIR)\stationapidemo.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "c:/rdp/include" /I "c:/rdp/rdp" /I "c:/rdp/dbgutil" /I ".." /I "../../crypt" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\stationapidemo.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\stationapidemo.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\stationapidemo.sbr"

"$(OUTDIR)\stationapidemo.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=dbgutil.lib rdpd.lib 989crypt.lib wsock32.lib winmm.lib stationapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\stationapidemo.pdb" /map:"$(INTDIR)\stationapidemo.map" /debug /machine:I386 /out:"$(OUTDIR)\stationapidemo.exe" /pdbtype:sept /libpath:"..\debug" /libpath:"c:\rdp\rdp" /libpath:"c:\rdp\dbgutil" /libpath:"..\..\crypt\debug" 
LINK32_OBJS= \
	"$(INTDIR)\stationapidemo.obj" \
	"..\Debug\stationapi.lib" \
	"..\..\tsip\Debug\tsip.lib" \
	"..\..\crypt\Debug\989crypt.lib"

"$(OUTDIR)\stationapidemo.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
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
!IF EXISTS("stationapidemo.dep")
!INCLUDE "stationapidemo.dep"
!ELSE 
!MESSAGE Warning: cannot find "stationapidemo.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "stationapidemo - Win32 Release" || "$(CFG)" == "stationapidemo - Win32 Debug"

!IF  "$(CFG)" == "stationapidemo - Win32 Release"

"stationapi - Win32 Release" : 
   cd "\Documents and Settings\Eric Hagstrom\Desktop\API\stationapi"
   $(MAKE) /$(MAKEFLAGS) /F ".\stationapi.mak" CFG="stationapi - Win32 Release" 
   cd ".\stationapidemo"

"stationapi - Win32 ReleaseCLEAN" : 
   cd "\Documents and Settings\Eric Hagstrom\Desktop\API\stationapi"
   $(MAKE) /$(MAKEFLAGS) /F ".\stationapi.mak" CFG="stationapi - Win32 Release" RECURSE=1 CLEAN 
   cd ".\stationapidemo"

!ELSEIF  "$(CFG)" == "stationapidemo - Win32 Debug"

"stationapi - Win32 Debug" : 
   cd "\Documents and Settings\Eric Hagstrom\Desktop\API\stationapi"
   $(MAKE) /$(MAKEFLAGS) /F ".\stationapi.mak" CFG="stationapi - Win32 Debug" 
   cd ".\stationapidemo"

"stationapi - Win32 DebugCLEAN" : 
   cd "\Documents and Settings\Eric Hagstrom\Desktop\API\stationapi"
   $(MAKE) /$(MAKEFLAGS) /F ".\stationapi.mak" CFG="stationapi - Win32 Debug" RECURSE=1 CLEAN 
   cd ".\stationapidemo"

!ENDIF 

!IF  "$(CFG)" == "stationapidemo - Win32 Release"

"tsip - Win32 Release" : 
   cd "\Documents and Settings\Eric Hagstrom\Desktop\API\tsip"
   $(MAKE) /$(MAKEFLAGS) /F ".\tsip.mak" CFG="tsip - Win32 Release" 
   cd "..\stationapi\stationapidemo"

"tsip - Win32 ReleaseCLEAN" : 
   cd "\Documents and Settings\Eric Hagstrom\Desktop\API\tsip"
   $(MAKE) /$(MAKEFLAGS) /F ".\tsip.mak" CFG="tsip - Win32 Release" RECURSE=1 CLEAN 
   cd "..\stationapi\stationapidemo"

!ELSEIF  "$(CFG)" == "stationapidemo - Win32 Debug"

"tsip - Win32 Debug" : 
   cd "\Documents and Settings\Eric Hagstrom\Desktop\API\tsip"
   $(MAKE) /$(MAKEFLAGS) /F ".\tsip.mak" CFG="tsip - Win32 Debug" 
   cd "..\stationapi\stationapidemo"

"tsip - Win32 DebugCLEAN" : 
   cd "\Documents and Settings\Eric Hagstrom\Desktop\API\tsip"
   $(MAKE) /$(MAKEFLAGS) /F ".\tsip.mak" CFG="tsip - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\stationapi\stationapidemo"

!ENDIF 

!IF  "$(CFG)" == "stationapidemo - Win32 Release"

"989crypt - Win32 Release" : 
   cd "\Documents and Settings\Eric Hagstrom\Desktop\API\crypt"
   $(MAKE) /$(MAKEFLAGS) /F ".\989crypt.mak" CFG="989crypt - Win32 Release" 
   cd "..\stationapi\stationapidemo"

"989crypt - Win32 ReleaseCLEAN" : 
   cd "\Documents and Settings\Eric Hagstrom\Desktop\API\crypt"
   $(MAKE) /$(MAKEFLAGS) /F ".\989crypt.mak" CFG="989crypt - Win32 Release" RECURSE=1 CLEAN 
   cd "..\stationapi\stationapidemo"

!ELSEIF  "$(CFG)" == "stationapidemo - Win32 Debug"

"989crypt - Win32 Debug" : 
   cd "\Documents and Settings\Eric Hagstrom\Desktop\API\crypt"
   $(MAKE) /$(MAKEFLAGS) /F ".\989crypt.mak" CFG="989crypt - Win32 Debug" 
   cd "..\stationapi\stationapidemo"

"989crypt - Win32 DebugCLEAN" : 
   cd "\Documents and Settings\Eric Hagstrom\Desktop\API\crypt"
   $(MAKE) /$(MAKEFLAGS) /F ".\989crypt.mak" CFG="989crypt - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\stationapi\stationapidemo"

!ENDIF 

SOURCE=.\stationapidemo.cpp

!IF  "$(CFG)" == "stationapidemo - Win32 Release"


"$(INTDIR)\stationapidemo.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "stationapidemo - Win32 Debug"


"$(INTDIR)\stationapidemo.obj"	"$(INTDIR)\stationapidemo.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 


!ENDIF 

