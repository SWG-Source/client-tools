TEMPLATE	= app
TARGET		= prodcons

CONFIG		+= qt warn_on

REQUIRES	= thread large-config

SOURCES		= prodcons.cpp
CLEAN_FILES	= prodcons.out
