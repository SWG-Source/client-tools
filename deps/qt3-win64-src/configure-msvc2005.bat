::
:: $Id: configure-msvc2005.bat 1341 2005-09-30 15:37:08Z chehrlic $
::
@echo off
echo Building Qt/Win Free for Microsoft Visual Studio 2005
set QMAKESPEC=win32-msvc2005
configure.bat %*
