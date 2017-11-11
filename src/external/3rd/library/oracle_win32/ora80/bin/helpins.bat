@echo off
REM
REM Copyright (c) Oracle Corporation 2000.  All Rights Reserved.
REM
REM NAME
REM   helpins.bat   
REM
REM DESCRIPTION
REM   Before running this script set the environment variable 
REM   SYSTEM_PASS to "system/pw" where pw is the password of the SYSTEM
REM   schema.
REM
REM USAGE
REM   helpins <language_specifier>
REM   e.g. To load help data in English run:
REM           helpins us

setlocal

REM Set SQL*Plus Default settings
set plushelp_loc=%ORACLE_HOME%\sqlplus\admin\help
set plusbld_file=helpbld.sql
set plusdata_file=helpus.sql
set lang=us

REM Check if ORACLE_HOME has been set
if (%ORACLE_HOME%) == () goto nohome

REM get language specifier
if not (%1) == () set lang=%1
set plusdata_file=help%lang%.sql


REM Check if help%lang% exists, if not, set the help file to US
if not (%lang%) == (us) (if not exist "%plushelp_loc%\%plusdata_file%" set lang=us)
set plusdata_file=help%lang%.sql


REM Check if SYSTEM_PASS has been set
if (%SYSTEM_PASS%) == () goto no_system_pass
set pluslogin_info=%system_pass%


REM Load the help
echo Loading HELP data from %plushell_loc%\%plusdata_file%
start %ORACLE_HOME%\bin\sqlplus %pluslogin_info% @%plushelp_loc%\%plusbld_file% %plushelp_loc%\%plusdata_file%

goto exit


:no_system_pass
echo Set SYSTEM_PASS in the environment, then restart help installation.
echo SYSTEM_PASS should be set to system/pw where pw is the password
echo of the SYSTEM schema.
goto exit

:nohome
echo ORACLE_HOME environment variable is not set

:exit
endlocal
