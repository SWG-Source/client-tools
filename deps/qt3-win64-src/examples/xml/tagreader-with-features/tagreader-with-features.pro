TEMPLATE	= app
TARGET          = tagreader-with-features

CONFIG		+= qt warn_on release

REQUIRES	= xml large-config

HEADERS		= structureparser.h
SOURCES		= tagreader.cpp \
                  structureparser.cpp
INTERFACES	=
