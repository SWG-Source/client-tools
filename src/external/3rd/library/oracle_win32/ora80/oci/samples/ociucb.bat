REM
REM Copyright (c) 1999 by Oracle Corporation. All Rights Reserved. 
REM Batch file for building UCB Demos
REM
@echo off
REM Place ucb dll in %oracle_home%\bin
set filename=%1
cl -I..\include -I. -D_DLL -D_MT %filename%.c /LD  /link /libpath:%oracle_home%\oci\lib\msvc oci.lib msvcrt.lib /nod:libc /nod:libcmt /out:%filename%.dll /entry:_DllMainCRTStartup@12 /EXPORT:%ORA_OCI_UCBPKG%EnvCallback /EXPORT:%ORA_OCI_UCBPKG%Init /Dll %2 %3 %4 %5
goto end

:usage
echo.
echo Usage: ociucb.bat filename [e.g. ociucb cdemoucb]
echo.
:end
