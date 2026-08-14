TEMPLATE = app
LANGUAGE = C++
TARGET	 = filechooser

SOURCES	+= filechooser.cpp main.cpp
HEADERS	+= filechooser.h
CONFIG	+= qt warn_on release
DBFILE	= filechooser.db
DEFINES += FILECHOOSER_IS_WIDGET
