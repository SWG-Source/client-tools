TEMPLATE	= app
TARGET          = tagreader

CONFIG		+= qt console warn_on release

REQUIRES	= xml large-config

HEADERS		= structureparser.h
SOURCES		= tagreader.cpp \
                  structureparser.cpp
INTERFACES	=
