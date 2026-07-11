::
:: $Id: configure-msvc.bat 858 2005-01-10 17:07:30Z atomice $
::
@echo off
echo Building Qt/Win Free for Microsoft Visual Studio 6.0
set QMAKESPEC=win32-msvc
configure.bat %*
