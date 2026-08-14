TEMPLATE	= app
TARGET		= tooltip

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

REQUIRES	= large-config

HEADERS		= tooltip.h
SOURCES		= main.cpp \
		  tooltip.cpp
