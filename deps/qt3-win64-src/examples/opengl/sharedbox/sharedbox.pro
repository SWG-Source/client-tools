TEMPLATE	= app
TARGET		= sharedbox

CONFIG		+= qt opengl warn_on release
CONFIG -= dlopen_opengl
DEPENDPATH	= ../include

REQUIRES        = opengl

HEADERS		= glbox.h \
		  globjwin.h
SOURCES		= glbox.cpp \
		  globjwin.cpp \
		  main.cpp
