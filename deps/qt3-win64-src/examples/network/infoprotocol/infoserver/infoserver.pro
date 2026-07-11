TEMPLATE	= app
TARGET		= infoserver

CONFIG		+= qt warn_on release

REQUIRES        = network full-config nocrosscompiler

HEADERS		= server.h \
		  infodata.h
SOURCES		= main.cpp \
		  server.cpp \
		  infodata.cpp
INTERFACES	= serverbase.ui
