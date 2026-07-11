::
:: $Id: configure-mingw.bat 858 2005-01-10 17:07:30Z atomice $
::
@echo off
echo Building Qt/Win Free for MinGW
set QMAKESPEC=win32-g++
configure.bat %*
