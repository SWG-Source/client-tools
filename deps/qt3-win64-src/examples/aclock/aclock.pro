TEMPLATE	= app
TARGET		= aclock

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

REQUIRES	= full-config

HEADERS		= aclock.h
SOURCES		= aclock.cpp \
		  main.cpp
