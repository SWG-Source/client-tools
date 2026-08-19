TEMPLATE	= app
TARGET		= glpixmap

CONFIG		+= qt opengl warn_on release
CONFIG -= dlopen_opengl
!mac:unix:LIBS  += -lm
DEPENDPATH	= ../include

REQUIRES         = opengl

HEADERS		= glbox.h \
		  globjwin.h
SOURCES		= glbox.cpp \
		  globjwin.cpp \
		  main.cpp
