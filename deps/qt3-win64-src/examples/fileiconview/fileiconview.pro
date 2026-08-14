TEMPLATE	= app
TARGET		= fileiconview

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

REQUIRES        = iconview full-config

HEADERS		= mainwindow.h \
		  qfileiconview.h \
		  ../dirview/dirview.h
SOURCES		= main.cpp \
		  mainwindow.cpp \
		  qfileiconview.cpp \
		  ../dirview/dirview.cpp
