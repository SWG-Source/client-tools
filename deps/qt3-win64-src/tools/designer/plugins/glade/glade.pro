TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin
HEADERS		= glade2ui.h
SOURCES		= main.cpp glade2ui.cpp
DESTDIR		= ../../../../plugins/designer
DEFINES 	+= QT_INTERNAL_XML
include( ../../../../src/qt_professional.pri )
TARGET		= gladeplugin
INCLUDEPATH	+= ../../interfaces


target.path += $$plugins.path/designer
INSTALLS 	+= target
