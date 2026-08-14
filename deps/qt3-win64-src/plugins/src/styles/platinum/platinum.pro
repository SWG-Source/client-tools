TEMPLATE = lib
TARGET	 = qplatinumstyle

CONFIG  += qt warn_off release plugin
DESTDIR  = ../../../styles

HEADERS		= ../../../../include/qplatinumstyle.h
SOURCES		= main.cpp \
		  ../../../../src/styles/qplatinumstyle.cpp

!contains(styles, windows) {
	HEADERS += ../../../../include/qwindowsstyle.h
	SOURCES += ../../../../src/styles/qwindowsstyle.cpp
}

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

target.path += $$plugins.path/styles
INSTALLS += target
