TEMPLATE	= app
TARGET		= mail

CONFIG		+= qt warn_on release

REQUIRES        = network full-config

HEADERS		= composer.h \
		  smtp.h
SOURCES		= composer.cpp \
		  main.cpp \
		  smtp.cpp
