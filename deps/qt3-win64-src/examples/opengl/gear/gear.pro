TEMPLATE	= app
TARGET		= gear

CONFIG		+= qt opengl warn_on release
CONFIG -= dlopen_opengl
!mac:unix:LIBS  += -lm
DEPENDPATH	= ../include

REQUIRES        = opengl

HEADERS		=
SOURCES		= gear.cpp
