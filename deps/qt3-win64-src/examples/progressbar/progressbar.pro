TEMPLATE	= app
TARGET		= progressbar

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

REQUIRES	= large-config

HEADERS		= progressbar.h
SOURCES		= main.cpp \
		  progressbar.cpp
