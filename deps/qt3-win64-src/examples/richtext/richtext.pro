TEMPLATE	= app
TARGET		= richtext

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

REQUIRES	= medium-config

HEADERS		= richtext.h
SOURCES		= main.cpp \
		  richtext.cpp
