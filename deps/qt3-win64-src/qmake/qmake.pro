TEMPLATE = app
CONFIG   = console release qtinc
TARGET  += qmake
DEFINES += QT_NO_TEXTCODEC QT_NO_UNICODETABLES QT_LITE_COMPONENT QT_NODLL QT_NO_STL QT_NO_COMPRESS HAVE_QCONFIG_CPP
INCLUDEPATH += . generators generators/unix generators/win32 generators/mac ..\include ..\include\private ..\include\qmake $(BCB)\include
DESTDIR = ../bin
OBJECTS_DIR = tmp
QMAKE_CLEAN += ..\bin\qmake.exe

win32-msvc {
	LIBS += ole32.lib uuid.lib advapi32.lib
	QMAKE_CLEAN += vc60.pdb qmake.pdb qmake.ilk ..\bin\qmake.exe
}

win32-msvc.net {
	LIBS += ole32.lib uuid.lib advapi32.lib
	QMAKE_CLEAN += vc60.pdb qmake.pdb qmake.ilk ..\bin\qmake.exe
}

win32-borland {
	QMAKE_CLEAN += ..\bin\qmake.tds ..\bin\qmake.exe
}

win32-g++ {
	LIBS += -lole32
}


# Input

SOURCES =  \
		generators/makefile.cpp \
		generators/projectgenerator.cpp \
		generators/mac/metrowerks_xml.cpp \
		generators/mac/pbuilder_pbx.cpp \
		generators/win32/borland_bmake.cpp \
		generators/win32/msvc_dsp.cpp \
		generators/win32/msvc_nmake.cpp \
		generators/win32/msvc_objectmodel.cpp \
		generators/win32/msvc_vcproj.cpp \
		generators/win32/mingw_make.cpp \
		generators/win32/winmakefile.cpp \
		generators/unix/unixmake.cpp \
		generators/unix/unixmake2.cpp \
		option.cpp \
		main.cpp \
		project.cpp \
		property.cpp \
		meta.cpp \
		qtmd5.cpp \
		../src/tools/qbitarray.cpp \
		../src/tools/qbuffer.cpp \
		../src/tools/qconfig.cpp \
		../src/tools/qcstring.cpp \
		../src/tools/qdatastream.cpp \
		../src/tools/qdatetime.cpp \
		../src/tools/qdir.cpp \
		../src/tools/qfile.cpp \
		../src/tools/qfileinfo.cpp \
		../src/tools/qgarray.cpp \
		../src/tools/qgcache.cpp \
		../src/tools/qgdict.cpp \
		../src/tools/qglist.cpp \
		../src/tools/qglobal.cpp \
		../src/tools/qgvector.cpp \
		../src/tools/qiodevice.cpp \
		../src/tools/qlibrary.cpp \
		../src/tools/qmap.cpp \
		../src/tools/qptrcollection.cpp \
		../src/tools/qregexp.cpp \
		../src/tools/qstring.cpp \
		../src/tools/qstringlist.cpp \
		../src/codecs/qtextcodec.cpp \
		../src/tools/qtextstream.cpp \
		../src/tools/quuid.cpp \
		../src/tools/qsettings.cpp \
		../src/tools/qlocale.cpp \
		../src/tools/qunicodetables.cpp


win32*: {
	SOURCES +=  \
		../src/tools/qfile_win.cpp \
		../src/tools/qfileinfo_win.cpp \
		../src/tools/qdir_win.cpp \
		../src/tools/qlibrary_win.cpp \
		../src/tools/qsettings_win.cpp
}

unix:!win32*: {
	SOURCES +=  \
		../src/tools/qfile_unix.cpp \
		../src/tools/qfileinfo_unix.cpp \
		../src/tools/qdir_unix.cpp \
		../src/tools/qlibrary_unix.cpp \
}

HEADERS =  \
		generators/makefile.h \
		generators/projectgenerator.h \
		generators/mac/metrowerks_xml.h \
		generators/mac/pbuilder_pbx.h \
		generators/unix/unixmake.h \
		generators/win32/borland_bmake.h \
		generators/win32/msvc_dsp.h \
		generators/win32/msvc_nmake.h \
		generators/win32/msvc_objectmodel.h \
		generators/win32/msvc_vcproj.h \
		generators/win32/winmakefile.h \
		option.h \
		project.h \
		property.h \
		meta.h \
		qtmd5.h

