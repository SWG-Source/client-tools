REM
REM Copyright (c) 1999 by Oracle Corporation. All Rights Reserved. 
REM
@echo off
if (%1) == () goto usage
if (%1) == (cdemo6) goto cplusplus
if (%1) == ("cdemo6") goto cplusplus
if (%1) == (CDEMO6) goto cplusplus
if (%1) == ("CDEMO6") goto cplusplus
if (%1) == (ociucb) goto callucb
if (%1) == ("ociucb") goto callucb
if (%1) == (OCIUCB) goto callucb
if (%1) == ("OCIUCB") goto callucb
if (%1) == (EXTDEMO2) goto dllmake
if (%1) == ("EXTDEMO2") goto dllmake
if (%1) == (extdemo2) goto dllmake
if (%1) == ("extdemo2") goto dllmake
if (%1) == (cdemodp) goto cdemodpmake
if (%1) == ("cdemodp") goto cdemodpmake
if (%1) == (CDEMODP) goto cdemodpmake
if (%1) == ("CDEMODP") goto cdemodpmake
if (%1) == (cdemodp_lip) goto cdemodpmake
if (%1) == ("cdemodp_lip") goto cdemodpmake
if (%1) == (CDEMODP_lip) goto cdemodpmake
if (%1) == ("CDEMODP_lip") goto cdemodpmake

cl -I%ORACLE_HOME%\oci\include -I. -D_DLL -D_MT %1.c /link /LIBPATH:%ORACLE_HOME%\oci\lib\msvc oci.lib kernel32.lib msvcrt.lib /nod:libc
goto end

:cplusplus
cl -I%ORACLE_HOME%\oci\include -I. -D_DLL -D_MT %1.cpp /link /LIBPATH:%ORACLE_HOME%\oci\lib\msvc oci.lib msvcrt.lib /nod:libc
goto end

:callucb
call ociucb.bat %1 
goto end

:dllmake
cl -I%ORACLE_HOME%\oci\include -I. -D_DLL -D_MT %1.c /link /Dll /out:%1l.dll /LIBPATH:%ORACLE_HOME%\oci\lib\msvc oci.lib kernel32.lib msvcrt.lib /nod:libc /export:qxiqtbi /export:qxiqtbd /export:qxiqtbu /export:qxiqtbs /export:qxiqtbf /export:qxiqtbc 
goto end

:cdemodpmake
cl -I%ORACLE_HOME%\oci\include -I. -D_DLL -D_MT cdemodp_lip.c cdemodp.c  /link /LIBPATH:%ORACLE_HOME%\oci\lib\msvc oci.lib kernel32.lib msvcrt.lib /nod:libc /out:cdemodp_lip.exe
goto end

:usage
echo.
echo Usage: make filename [i.e. make oci01]
echo.
:end
