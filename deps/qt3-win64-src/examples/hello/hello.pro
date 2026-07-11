TEMPLATE	= app
TARGET		= hello

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

HEADERS		= hello.h
SOURCES		= hello.cpp \
		  main.cpp
