TEMPLATE	= app
TARGET		= widgets

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include
INCLUDEPATH	+= ../aclock ../dclock

REQUIRES	= full-config

HEADERS		= widgets.h ../aclock/aclock.h ../dclock/dclock.h
SOURCES		= main.cpp widgets.cpp ../aclock/aclock.cpp ../dclock/dclock.cpp
