TEMPLATE	= app
CONFIG		+= qt warn_on console
HEADERS		= ../shared/metatranslator.h \
		  ../shared/proparser.h
SOURCES		= fetchtr.cpp \
		  main.cpp \
		  merge.cpp \
		  numberh.cpp \
		  sametexth.cpp \
		  ../shared/metatranslator.cpp \
		  ../shared/proparser.cpp

DEFINES 	+= QT_INTERNAL_XML
include( ../../../src/qt_professional.pri )

TARGET		= lupdate
INCLUDEPATH	+= ../shared
DESTDIR		= ../../../bin

target.path=$$bins.path
INSTALLS	+= target
