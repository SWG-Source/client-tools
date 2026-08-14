TEMPLATE	= app
TARGET		= rangecontrols

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

REQUIRES	= large-config

HEADERS		= rangecontrols.h
SOURCES		= main.cpp \
		  rangecontrols.cpp
