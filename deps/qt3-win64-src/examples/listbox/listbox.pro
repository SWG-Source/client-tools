TEMPLATE	= app
TARGET		= listbox

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

REQUIRES	= large-config

HEADERS		= listbox.h
SOURCES		= listbox.cpp \
		  main.cpp
