@echo off
REM
REM This script creates the Product User Profile tables for SQL*Plus
REM
REM Before running this script set the environment variable 
REM SYSTEM_PASS to "system/pw" where pw is the password of the SYSTEM
REM schema.
REM
REM Usage: pupbld


setlocal

REM Set SQL*Plus Default settings
set pupbld_file=%ORACLE_HOME%\sqlplus\admin\pupbld.sql

REM Check if SYSTEM_PASS has been set
if (%SYSTEM_PASS%) == () goto no_system_pass

REM Install the PUP tables
REM Note user must enter EXIT to quit
echo Installing Product User Profile tables
start %ORACLE_HOME%\bin\sqlplus %SILENT% %SYSTEM_PASS% @%pupbld_file%

goto exit

 
:no_system_pass
echo Set SYSTEM_PASS in the environment, then restart PUP installation.
echo SYSTEM_PASS should be set to system/pw where pw is the password
echo of the SYSTEM schema.

:exit
endlocal
