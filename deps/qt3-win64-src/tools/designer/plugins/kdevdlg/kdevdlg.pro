TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin
HEADERS		= kdevdlg2ui.h
SOURCES		= main.cpp kdevdlg2ui.cpp
DESTDIR		= ../../../../plugins/designer
TARGET		= kdevdlgplugin
INCLUDEPATH	+= ../../interfaces


target.path += $$plugins.path/designer
INSTALLS 	+= target
