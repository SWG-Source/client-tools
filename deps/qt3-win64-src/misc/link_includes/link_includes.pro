TEMPLATE 	= app
CONFIG   	= console release qtinc no_autoqmake
TARGET  	+= link_includes
DEFINES 	+= QT_NO_TEXTCODEC QT_NO_UNICODETABLES QT_LITE_COMPONENT QT_NODLL QT_NO_STL QT_NO_COMPRESS
INCLUDEPATH 	+= $(QTDIR)\src\tools $(QTDIR)\src\kernel $(QTDIR)\include $(QTDIR)\include\private $(QTDIR)\qmake
DESTDIR 	= $(QTDIR)/bin
OBJECTS_DIR	= tmp

win32-msvc {
        LIBS += ole32.lib
        QMAKE_CLEAN += vc60.pdb link_includes.pdb link_includes.ilk $(QTDIR)\bin\link_includes.exe
}

win32-borland {
        QMAKE_CLEAN += $(QTDIR)\bin\link_includes.tds $(QTDIR)\bin\link_includes.exe
        INCLUDEPATH 	+= $(BCB)\include
}

win32-g++ {
        LIBS += -lole32
}

# is this really all needed?
HEADERS +=      $(QTDIR)/qmake/qtmd5.h
SOURCES +=	link_includes.cpp \
                $(QTDIR)/qmake/qtmd5.cpp \
		$(QTDIR)/src/tools/qbitarray.cpp \
		$(QTDIR)/src/tools/qbuffer.cpp \
		$(QTDIR)/src/tools/qcstring.cpp \
		$(QTDIR)/src/tools/qdatastream.cpp \
		$(QTDIR)/src/tools/qdatetime.cpp \
		$(QTDIR)/src/tools/qdir.cpp \
		$(QTDIR)/src/tools/qdir_win.cpp \
		$(QTDIR)/src/tools/qfile.cpp \
		$(QTDIR)/src/tools/qfile_win.cpp \
		$(QTDIR)/src/tools/qfileinfo.cpp \
		$(QTDIR)/src/tools/qfileinfo_win.cpp \
		$(QTDIR)/src/tools/qgarray.cpp \
		$(QTDIR)/src/tools/qgcache.cpp \
		$(QTDIR)/src/tools/qgdict.cpp \
		$(QTDIR)/src/tools/qglist.cpp \
		$(QTDIR)/src/tools/qglobal.cpp \
		$(QTDIR)/src/tools/qgvector.cpp \
		$(QTDIR)/src/tools/qiodevice.cpp \
		$(QTDIR)/src/tools/qlocale.cpp \
		$(QTDIR)/src/tools/qmap.cpp \
		$(QTDIR)/src/tools/qptrcollection.cpp \
		$(QTDIR)/src/tools/qregexp.cpp \
		$(QTDIR)/src/tools/qstring.cpp \
		$(QTDIR)/src/tools/qstringlist.cpp \
		$(QTDIR)/src/tools/qunicodetables.cpp 
		
