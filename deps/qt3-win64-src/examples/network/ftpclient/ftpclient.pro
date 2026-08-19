TEMPLATE	= app
TARGET		= ftpclient

CONFIG		+= qt warn_on release

REQUIRES	= network full-config nocrosscompiler

HEADERS		= ftpviewitem.h
SOURCES		= main.cpp \
		  ftpviewitem.cpp
FORMS		= ftpmainwindow.ui \
		  connectdialog.ui
IMAGES		= images/file.png \
		  images/folder.png
