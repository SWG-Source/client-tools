@echo off
REM
REM Copyright (c) 1999 by Oracle Corporation. All Rights Reserved. 
REM Batch file for building UCB Demos
REM
if (%BORLAND_HOME%) == () goto nobchome

set BINC=%BORLAND_HOME%\include
set BLIB=%BORLAND_HOME%\lib

REM Place ucb dll in %oracle_home%\bin
%BORLAND_HOME%\bin\bcc32 -c -a4 -DOCI_BORLAND -I%BINC% -I. -I..\include %1.c
echo LIBRARY %1.dll > %1.def
echo DESCRIPTION '%1.dll' >> %1.def
echo EXPORTS >> %1.def
echo _OCIEnvCallback=OCIEnvCallback @1>> %1.def
echo _kpucbFuncs=kpucbFuncs @2>> %1.def
echo _kpucbMain=kpucbMain @3>> %1.def
%BORLAND_HOME%\bin\bcc32 -tWD -L%BLIB%  %1.obj ..\lib\bc\oci.lib bidsfi.lib cw32i.lib 
REM /EXPORT:OCIEnvCallback /EXPORT:kpucbFuncs /EXPORT:kpucbMain /Dll
goto end

:nobchome
echo .
echo Please set environment variable BORLAND_HOME
echo .
goto end
:usage
echo.
echo Usage: ociucb.bat filename [e.g. ociucb cdemoucb]
echo.
:end
