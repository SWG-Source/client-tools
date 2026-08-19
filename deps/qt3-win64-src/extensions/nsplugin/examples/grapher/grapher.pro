TEMPLATE	= lib
TARGET		= grapher
win32:TARGET	= npgrapher

CONFIG	       += qt dll release
LIBS	       += -lqnp
unix:LIBS      += -lXt

HEADERS		=
SOURCES		= grapher.cpp
DEF_FILE	= grapher.def
RC_FILE		= grapher.rc
