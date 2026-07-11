TEMPLATE	= app
TARGET		= moc

CONFIG 		= console release qtinc yacc lex_included yacc_no_name_mangle
DEFINES	       += QT_MOC QT_NO_CODECS QT_LITE_UNICODE QT_NO_COMPONENT \
		  QT_NO_STL QT_NO_COMPRESS
win32:DEFINES  += QT_NODLL YY_NO_UNISTD_H
DESTDIR         = ../../bin


INCLUDEPATH	+= $$QT_SOURCE_TREE/include ../tools .
DEPENDPATH	+= $$QT_SOURCE_TREE/include ../tools .
LIBS		=
OBJECTS_DIR	= .
SOURCES		= ../tools/qbuffer.cpp	    \
		  ../tools/qptrcollection.cpp  \
		  ../tools/qcstring.cpp	    \
		  ../tools/qdatastream.cpp  \
		  ../tools/qdatetime.cpp    \
		  ../tools/qfile.cpp	    \
		  ../tools/qdir.cpp	    \
		  ../tools/qfileinfo.cpp    \
		  ../tools/qgarray.cpp	    \
		  ../tools/qgdict.cpp	    \
		  ../tools/qglist.cpp	    \
		  ../tools/qglobal.cpp	    \
		  ../tools/qgvector.cpp	    \
		  ../tools/qiodevice.cpp    \
		  ../tools/qregexp.cpp	    \
		  ../tools/qstring.cpp	    \
		  ../tools/qlocale.cpp	    \
                  ../tools/qunicodetables.cpp \
		  ../tools/qstringlist.cpp  \
		  ../tools/qtextstream.cpp  \
		  ../tools/qbitarray.cpp    \
		  ../tools/qmap.cpp         \
		  ../tools/qgcache.cpp      \
		  ../codecs/qtextcodec.cpp \
		  ../codecs/qutfcodec.cpp

isEmpty(QT_PRODUCT)|contains(QT_PRODUCT, qt-internal) {
    LEXSOURCES  = moc.l
    YACCSOURCES = moc.y
} else {
    SOURCES   += moc_yacc.cpp
}

unix:SOURCES	+= ../tools/qfile_unix.cpp ../tools/qdir_unix.cpp ../tools/qfileinfo_unix.cpp
win32:SOURCES	+= ../tools/qfile_win.cpp ../tools/qdir_win.cpp ../tools/qfileinfo_win.cpp
macx:LIBS	+= -framework Carbon

target.path=$$bins.path
INSTALLS += target

*-mwerks {
   TEMPLATE = lib
   TARGET = McMoc
   CONFIG -= static
   CONFIG += shared plugin
   DEFINES += MOC_MWERKS_PLUGIN
   MWERKSDIR = $QT_SOURCE_TREE/util/mwerks_plugin
   INCLUDEPATH += $$MWERKSDIR/Headers
   LIBS += $$MWERKSDIR/Libraries/PluginLib4.shlb
   SOURCES += mwerks_mac.cpp
}
