::
:: $Id: configure-borland.bat 858 2005-01-10 17:07:30Z atomice $
::
@echo off
echo Building Qt/Win Free for Borland CBuilder 5.x
set QMAKESPEC=win32-borland
configure.bat %*
