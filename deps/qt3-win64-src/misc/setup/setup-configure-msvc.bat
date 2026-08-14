::
:: msvc
cd ..\..
set QTDIR=%CD%
set QMAKESPEC=win32-msvc
call configure.bat -nobuild -shared -fast -thread -stl -zlib -opengl -gif -qt-imgfmt-png -qt-imgfmt-jpeg -qt-imgfmt-mng -no-ipv6 -pch
cd src\moc
nmake -f Makefile
cd ..
nmake -f Makefile.main
nmake -f Makefile
cd ..\tools
nmake -f Makefile
cd ..
cd misc\setup
