TEMPLATE	= app
TARGET		= networkprotocol

CONFIG		+= qt warn_on release

REQUIRES        = network full-config

HEADERS		= nntp.h view.h
SOURCES		= main.cpp \
		  nntp.cpp view.cpp
