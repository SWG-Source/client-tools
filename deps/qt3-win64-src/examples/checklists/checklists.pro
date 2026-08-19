TEMPLATE	= app
TARGET		= checklists

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

REQUIRES	= large-config

HEADERS		= checklists.h
SOURCES		= checklists.cpp \
		  main.cpp
