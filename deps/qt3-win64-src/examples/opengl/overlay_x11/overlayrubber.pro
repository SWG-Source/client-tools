TEMPLATE	= app
TARGET		= overlayrubber

CONFIG		+= qt opengl warn_on release
CONFIG -= dlopen_opengl
DEPENDPATH	= ../include

REQUIRES        = opengl full-config

HEADERS		= gearwidget.h \
		  rubberbandwidget.h
SOURCES		= gearwidget.cpp \
		  main.cpp \
		  rubberbandwidget.cpp
