TEMPLATE	= app
TARGET		= texture

CONFIG		+= qt opengl warn_on release
CONFIG -= dlopen_opengl
DEPENDPATH	= ../include

REQUIRES        = opengl

HEADERS		= gltexobj.h \
		  globjwin.h
SOURCES		= gltexobj.cpp \
		  globjwin.cpp \
		  main.cpp
