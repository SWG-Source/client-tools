TEMPLATE	= app
TARGET		= customlayout

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

REQUIRES	= large-config

HEADERS		= border.h \
		  card.h \
		  flow.h
SOURCES		= border.cpp \
		  card.cpp \
		  flow.cpp \
		  main.cpp
