TEMPLATE	= app
TARGET		= remotecontrol

CONFIG		+= qt warn_on release

REQUIRES        = network full-config nocrosscompiler

HEADERS		= startup.h \
		  remotectrlimpl.h \
		  ipcserver.h
SOURCES		= main.cpp \
		  startup.cpp \
		  remotectrlimpl.cpp \
		  ipcserver.cpp
INTERFACES	= remotectrl.ui \
		  maindialog.ui
