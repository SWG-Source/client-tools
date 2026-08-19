TEMPLATE	= app
TARGET		= qfd

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include
REQUIRES	= full-config

HEADERS		= fontdisplayer.h
SOURCES		= fontdisplayer.cpp \
		  qfd.cpp
