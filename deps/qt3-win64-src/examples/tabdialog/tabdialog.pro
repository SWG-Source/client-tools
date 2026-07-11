TEMPLATE	= app
TARGET		= tabdialog

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

REQUIRES	= large-config

HEADERS		= tabdialog.h
SOURCES		= main.cpp \
		  tabdialog.cpp
