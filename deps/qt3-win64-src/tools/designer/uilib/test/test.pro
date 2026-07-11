TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		=
SOURCES		= main.cpp
TARGET		= test
LIBS		+=  -L$$QT_BUILD_TREE/lib -lqui

