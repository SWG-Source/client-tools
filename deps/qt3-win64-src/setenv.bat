::
:: $Id: setenv.bat 1200 2005-04-20 22:15:22Z habacker $
::
@echo off

set QTDIR=%CD%
if %QMAKESPEC% == win32-g++ (
	set PATH=\Mingw\bin;%PATH%
)
set PATH=%QTDIR%\bin;%PATH%
