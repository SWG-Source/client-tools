TEMPLATE = lib
TARGET  += qtwidgets

CONFIG  += qt dll plugin
DESTDIR  = ../../../accessible
VERSION  = 1.0.0

REQUIRES += accessibility

SOURCES  += main.cpp \
	    qaccessiblewidget.cpp \
	    qaccessiblemenu.cpp

HEADERS  += qaccessiblewidget.h \
	    qaccessiblemenu.h
