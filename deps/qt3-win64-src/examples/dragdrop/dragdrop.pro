TEMPLATE	= app
TARGET		= dragdrop

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

REQUIRES	= full-config

HEADERS		= dropsite.h \
		  secret.h
SOURCES		= dropsite.cpp \
		  main.cpp \
		  secret.cpp
