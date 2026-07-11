TEMPLATE	= app
TARGET		= infoclient

CONFIG		+= qt warn_on release

REQUIRES        = network full-config nocrosscompiler

HEADERS		= client.h
SOURCES		= main.cpp \
		  client.cpp
INTERFACES	= clientbase.ui
