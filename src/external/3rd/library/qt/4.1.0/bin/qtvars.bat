@echo off
rem
rem This file is generated
rem

echo Setting up a Qt environment...
echo -- QTDIR set to C:\Qt\4.1.0
echo -- Added C:\Qt\4.1.0\bin to PATH
echo -- QMAKESPEC set to win32-msvc

set QTDIR=C:\Qt\4.1.0
set PATH=C:\Qt\4.1.0\bin;%PATH%
set QMAKESPEC=win32-msvc

if not "%1"=="setup" goto SETUPDONE
cd %QTDIR%\examples
qmake -r -tp vc
:SETUPDONE

if not "%1"=="vsvars" goto END
call "C:\Program Files\Microsoft Visual Studio\VC98\bin\vcvars32.bat"
:END

