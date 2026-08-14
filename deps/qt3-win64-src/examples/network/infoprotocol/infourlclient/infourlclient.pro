TEMPLATE	= app
TARGET		= infourlclient

CONFIG		+= qt warn_on release

REQUIRES        = network full-config nocrosscompiler

HEADERS		= client.h \
		  qip.h
SOURCES		= main.cpp \
		  client.cpp \
		  qip.cpp
INTERFACES	= clientbase.ui
