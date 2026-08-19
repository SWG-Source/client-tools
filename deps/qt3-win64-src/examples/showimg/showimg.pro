TEMPLATE	= app
TARGET		= showimg

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

REQUIRES	= full-config

HEADERS		= showimg.h imagetexteditor.h \
		  imagefip.h
SOURCES		= main.cpp \
		  imagetexteditor.cpp \
		  showimg.cpp \
		  imagefip.cpp
