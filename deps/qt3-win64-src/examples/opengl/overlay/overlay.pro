TEMPLATE	= app
TARGET		= overlay

CONFIG		+= qt opengl warn_on release
CONFIG -= dlopen_opengl
DEPENDPATH	= ../include

REQUIRES        = opengl

HEADERS		= glteapots.h \
		  globjwin.h
SOURCES		= glteapots.cpp \
		  globjwin.cpp \
		  main.cpp
