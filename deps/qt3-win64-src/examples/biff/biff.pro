TEMPLATE	= app
TARGET		= biff

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

REQUIRES	= full-config

HEADERS		= biff.h
SOURCES		= biff.cpp \
		  main.cpp
