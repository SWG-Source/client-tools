TEMPLATE	= app
TARGET		= tictac

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

REQUIRES	= large-config

HEADERS		= tictac.h
SOURCES		= main.cpp \
		  tictac.cpp
