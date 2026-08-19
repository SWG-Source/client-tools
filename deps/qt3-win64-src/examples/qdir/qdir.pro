TEMPLATE	= app
TARGET		= qdir

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

REQUIRES	= full-config

HEADERS		= qdir.h ../dirview/dirview.h
SOURCES		= qdir.cpp ../dirview/dirview.cpp
