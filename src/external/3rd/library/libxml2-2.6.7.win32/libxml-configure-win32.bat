REM Copy this into the libxml*\win32 directory, then run it.
REM It will generate the libxml2 debug and release distribution we use for SWG
REM in d:\temp\libxml-build\debug and d:\temp\libxml-build\release

@REM debug library configuration and build
cscript configure.js ftp=no http=no html=yes c14n=no docb=no iconv=no xml_debug=yes mem_debug=yes sax1=yes legacy=no cruntime=/MTd prefix=d:\temp\libxml-build\debug debug=yes
nmake /f Makefile.msvc clean
nmake /f Makefile.msvc
nmake /f Makefile.msvc install

@REM release library configuration and build
cscript configure.js ftp=no http=no html=yes c14n=no docb=no iconv=no xml_debug=no mem_debug=no sax1=yes legacy=no cruntime=/MT prefix=d:\temp\libxml-build\release debug=no
nmake /f Makefile.msvc clean
nmake /f Makefile.msvc
nmake /f Makefile.msvc install
