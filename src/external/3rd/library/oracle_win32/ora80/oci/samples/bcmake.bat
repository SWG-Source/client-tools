@echo off
REM
REM Copyright (c) 1999 by Oracle Corporation. All Rights Reserved. \
REM $
REM
if (%BORLAND_HOME%) == () goto nobchome

set BINC=%BORLAND_HOME%\include 
set BLIB=%BORLAND_HOME%\lib
if (%1) == () goto usage

if (%1) == (cdemo6) goto cplusplus
if (%1) == (CDEMO6) goto cplusplus
if (%1) == (ociucb) goto callucb
if (%1) == (OCIUCB) goto callucb
if (%1) == (EXTDEMO2) goto dllmake
if (%1) == (extdemo2) goto dllmake
if (%1) == (cdemodp) goto cdemodpmake
if (%1) == (CDEMODP) goto cdemodpmake
if (%1) == (cdemodp_lip) goto cdemodpmake
if (%1) == (CDEMODP_lip) goto cdemodpmake

%BORLAND_HOME%\bin\bcc32 -w-pro -c -a4 -DOCI_BORLAND -I. -I%BINC% -I..\include %1.c
%BORLAND_HOME%\bin\bcc32 -L%BLIB% -L..\lib\bc %1.obj oci.lib bidsfi.lib 
goto end
:cplusplus
%BORLAND_HOME%\bin\bcc32 -w-pro -c -a4 -DOCI_BORLAND -I. -I%BINC% -I.\ -I..\include %1.cpp
%BORLAND_HOME%\bin\bcc32 -L%BLIB% -L..\lib\bc %1.obj oci.lib bidsfi.lib 
goto end

:nobchome
echo .
echo Please set environment variable BORLAND_HOME
echo .
goto end

:dllmake
%BORLAND_HOME%\bin\bcc32 -w-pro -c -a4 -DOCI_BORLAND -I. -I%BINC% -I..\include %1.c
echo LIBRARY extdemo2.dll > extdemo2.def
echo DESCRIPTION 'extdemo2.dll' >> extdemo2.def
echo EXPORTS >> extdemo2.def
echo _qxiqtbi=qxiqtbi >> extdemo2.def
echo _qxiqtbd=qxiqtbd >> extdemo2.def
echo _qxiqtbu=qxiqtbu >> extdemo2.def
echo _qxiqtbs=qxiqtbs >> extdemo2.def
echo _qxiqtbf=qxiqtbf >> extdemo2.def
echo _qxiqtbc=qxiqtbc >> extdemo2.def
%BORLAND_HOME%\bin\bcc32 -tWD -L%BLIB% -L..\lib\bc %1.obj oci.lib bidsfi.lib 
REM /export:qxiqtbi /export:qxiqtbd /export:qxiqtbu /export:qxiqtbs /export:qxiqtbf /export:qxiqtbc
goto end

:cdemodpmake
%BORLAND_HOME%\bin\bcc32 -w-pro -c -a4 -DOCI_BORLAND -I%BINC% -I. -I..\include cdemodp_lip.c cdemodp.c
%BORLAND_HOME%\bin\bcc32 -L%BLIB% -L..\lib\bc cdemodp_lip.obj cdemodp.obj oci.lib bidsfi.lib 
goto end

:callucb
call bcociucb.bat %1
goto end

:usage
echo.
echo Usage: make filename [i.e. make oci01]
echo.
:end
set BINC=
set BLIB=
