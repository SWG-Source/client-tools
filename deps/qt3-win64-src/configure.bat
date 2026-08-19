::
:: $Id: configure.bat 1341 2005-09-30 15:37:08Z chehrlic $
::
@echo off

:: setup some vars 
if not "%QTDIR%"=="" (
	if not exist "%QTDIR%\configure.bat" (
		echo Please set %%QTDIR%% to the Qt source directory.
		goto ERROR
	)
) else (
	if exist configure.bat (
		set QTDIR=%CD%
	) else (
		echo Please run configure again from the Qt source directory.
		goto ERROR
	)
)
set ERR=0
set MAKE=make
if "%QMAKESPEC%"=="" (
	:: Borland is the fastest compiler
	"%QTDIR%\misc\findpath\findpath" bcc32.exe >nul
	if errorlevel 1 (
		rem
	) else (
		set QMAKESPEC=win32-borland
		goto found
	)
	:: followed by Visual C++	
	%QTDIR%\misc\findpath\findpath cl.exe >nul
	if errorlevel 1 (
		rem
	) else (
		set QMAKESPEC=win32-msvc
		goto found
	)
	:: then GNU C++	
	"%QTDIR%\misc\findpath\findpath" g++.exe >nul
	if errorlevel 1 (
		rem
	) else (
		set QMAKESPEC=win32-g++
		goto found
	)
	echo Could not find a compiler to use.
	echo.
	echo Supported compilers:
	echo Borland C++             bcc32.exe
	echo Microsoft Visual C++    cl.exe
	echo GNU C++                 g++.exe
	goto ERROR
)
:found
cd %QTDIR%

if %QMAKESPEC% == win32-msvc set MAKE=nmake /NOLOGO
if %QMAKESPEC% == win32-msvc.net set MAKE=nmake /NOLOGO 
if %QMAKESPEC% == win32-msvc2005 set MAKE=nmake /NOLOGO 
if %QMAKESPEC% == win32-g++ (
	:: MinGW and cygwin does not work, look for sh.exe 	
	"%QTDIR%\misc\findpath\findpath" sh.exe >nul
	if errorlevel 1 (
		set MAKE=mingw32-make
	) else (
		echo. 
		echo MinGW does not work with cygwin -mingw32-make would use sh.exe- so remove 
		echo \cygwin\bin from your PATH variable or rename temporary your cygwin folder.
		goto ERROR
	) 	
)
if %QMAKESPEC% == win32-borland (
	:: Use bmake if the CBuilderX update is installed, else make
	"%QTDIR%\misc\findpath\findpath" bmake.exe >nul
	if errorlevel 1 (
		set MAKE=make
	) else (
		set MAKE=bmake
	)
)

set MAKEFILE=Makefile.%QMAKESPEC%

if %QMAKESPEC% == win32-msvc.net set MAKEFILE=Makefile.win32-msvc
if %QMAKESPEC% == win32-msvc2005 set MAKEFILE=Makefile.win32-msvc

:: 
echo install headers for bootstrapping...
IF NOT EXIST include\qconfig.h copy misc\qconfig.h include
IF NOT EXIST include\qmodules.h copy misc\qmodules.h include

echo building header copy tool...
cd misc\link_includes
IF NOT EXIST tmp  mkdir tmp
%MAKE% -f %MAKEFILE%

if ERRORLEVEL 1 set ERR=1
cd ..\..
if %ERR% == 1 goto ERROR

echo Copying header files...
::bin\link_includes %*
bin\link_includes

echo building configure.exe ...
cd misc\configure
IF NOT EXIST tmp  mkdir tmp
%MAKE% -f %MAKEFILE%
if ERRORLEVEL 1 set ERR=1
cd ..\..
if %ERR% == 1 goto ERROR

:: create temp dir for qmake 
IF NOT EXIST qmake\tmp  mkdir qmake\tmp

:: setup generic makefiles 
copy "qmake\%MAKEFILE%" "qmake\Makefile" 
copy "%MAKEFILE%" "Makefile"

bin\configure %* 

goto LEAVE

:ERROR
echo.
echo An error occured. Configuration aborted.
echo.

:LEAVE
SET ERR=

