TEMPLATE	= app
TARGET		= fontdemo

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

REQUIRES 	= full-config

HEADERS		= viewer.h
SOURCES		= simple-qfont-demo.cpp \
	          viewer.cpp
