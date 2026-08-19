TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= cannon.h \
		  gamebrd.h \
		  lcdrange.h
SOURCES		= cannon.cpp \
		  gamebrd.cpp \
		  lcdrange.cpp \
		  main.cpp
TARGET		= t13
REQUIRES=full-config
unix:LIBS += -lm

