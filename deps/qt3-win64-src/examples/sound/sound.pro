TEMPLATE	= app
TARGET		= sound

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

REQUIRES	= full-config
x11:REQUIRES	= nas

HEADERS		= sound.h
SOURCES		= sound.cpp
