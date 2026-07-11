TEMPLATE	= app
TARGET		= lineedits

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

REQUIRES	= medium-config

HEADERS		= lineedits.h
SOURCES		= lineedits.cpp \
		  main.cpp
