TEMPLATE = lib
TARGET	 = qwindowsstyle

CONFIG  += qt warn_off release plugin
DESTDIR	 = ../../../styles

HEADERS		= ../../../../include/qwindowsstyle.h
SOURCES		= main.cpp \
		  ../../../../src/styles/qwindowsstyle.cpp

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

target.path += $$plugins.path/styles
INSTALLS += target
