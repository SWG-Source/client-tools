::
:: borland
cd ..\..
set QTDIR=%CD%
set QMAKESPEC=win32-borland
call configure.bat -nobuild -shared -fast -thread -stl -zlib -opengl -gif -qt-imgfmt-png -qt-imgfmt-jpeg -qt-imgfmt-mng -no-ipv6 -pch
cd src\moc
make -f Makefile
cd ..
make -f Makefile.main
make -f Makefile
cd ..\tools
make -f Makefile
cd ..
cd misc\setup
