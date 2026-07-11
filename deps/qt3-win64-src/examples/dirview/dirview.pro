TEMPLATE	= app
TARGET		= dirview

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

REQUIRES	= full-config

HEADERS		= dirview.h
SOURCES		= dirview.cpp \
		  main.cpp
