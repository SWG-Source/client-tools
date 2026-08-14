TEMPLATE = lib
TARGET	 = qcompactstyle

CONFIG  += qt warn_off release plugin
DESTDIR	 = ../../../styles

HEADERS		= ../../../../include/qcompactstyle.h

SOURCES		= main.cpp \
		  ../../../../src/styles/qcompactstyle.cpp

!contains(styles, windows) {
	HEADERS += ../../../../include/qwindowsstyle.h
	SOURCES += ../../../../src/styles/qwindowsstyle.cpp
}

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

target.path += $$plugins.path/styles
INSTALLS += target
