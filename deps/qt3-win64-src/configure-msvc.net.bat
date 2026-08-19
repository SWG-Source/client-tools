::
:: $Id: configure-msvc.net.bat 858 2005-01-10 17:07:30Z atomice $
::
@echo off
echo Building Qt/Win Free for Microsoft Visual Studio .NET
set QMAKESPEC=win32-msvc.net
configure.bat %*
