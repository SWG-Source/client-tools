TEMPLATE = app
LANGUAGE = C++
TARGET	 = credit

SOURCES	+= main.cpp
HEADERS	+= creditform.h
SOURCES += creditform.cpp
FORMS	= creditformbase.ui
CONFIG	+= qt warn_on release
DBFILE	= credit.db
IMAGEFILE	= images.cpp
