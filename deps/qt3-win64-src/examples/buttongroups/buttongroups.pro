TEMPLATE	= app
TARGET		= buttongroups

CONFIG		+= qt warn_on release

REQUIRES	= small-config

HEADERS		= buttongroups.h
SOURCES		= buttongroups.cpp \
		  main.cpp
