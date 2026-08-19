TEMPLATE	= app
TARGET		= action

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

REQUIRES	= full-config

HEADERS		= application.h
SOURCES		= application.cpp \
		  main.cpp
