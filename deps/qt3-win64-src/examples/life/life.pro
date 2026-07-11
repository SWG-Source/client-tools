TEMPLATE	= app
TARGET		= life

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

REQUIRES	= medium-config

HEADERS		= life.h \
		  lifedlg.h
SOURCES		= life.cpp \
		  lifedlg.cpp \
		  main.cpp
