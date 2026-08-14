TEMPLATE	= app
TARGET		= qwerty

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

REQUIRES	= large-config

HEADERS		= qwerty.h
SOURCES		= main.cpp \
		  qwerty.cpp
