TEMPLATE	= app
CONFIG		+= qt console warn_on release professional
HEADERS	= uic.h \
		  ../shared/widgetdatabase.h \
		  ../shared/domtool.h \
		  ../shared/parser.h \
		  ../interfaces/widgetinterface.h

#HEADERS +=	  ../shared/ui2uib.h \
#		  ../shared/uib.h

SOURCES	= main.cpp uic.cpp form.cpp object.cpp \
		   subclassing.cpp embed.cpp\
		  ../shared/widgetdatabase.cpp  \
		  ../shared/domtool.cpp \
		  ../shared/parser.cpp

#SOURCES +=	  ../shared/ui2uib.cpp \
#		  ../shared/uib.cpp

DEFINES		+= QT_INTERNAL_XML
include( ../../../src/qt_professional.pri )

TARGET		= uic
INCLUDEPATH	+= ../shared
DEFINES 	+= UIC
DESTDIR		= ../../../bin

target.path=$$bins.path
INSTALLS        += target

*-mwerks {
   TEMPLATE = lib
   TARGET = McUic
   CONFIG -= static
   CONFIG += shared plugin
   DEFINES += UIC_MWERKS_PLUGIN
   MWERKSDIR = $(QT_SOURCE_TREE)/util/mwerks_plugin
   INCLUDEPATH += $$MWERKSDIR/Headers
   LIBS += $$MWERKSDIR/Libraries/PluginLib4.shlb
   SOURCES += mwerks_mac.cpp
}
