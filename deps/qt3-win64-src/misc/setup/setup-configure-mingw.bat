::
:: mingw
cd ..\..
set QTDIR=%CD%
set QMAKESPEC=win32-g++
call configure.bat -nobuild -shared -fast -thread -stl -zlib -opengl -gif -qt-imgfmt-png -qt-imgfmt-jpeg -qt-imgfmt-mng -no-ipv6 -pch
cd src\moc
mingw32-make -f Makefile
cd ..
mingw32-make -f Makefile.main
mingw32-make -f Makefile
cd ..\tools
mingw32-make -f Makefile
cd ..
cd misc\setup
