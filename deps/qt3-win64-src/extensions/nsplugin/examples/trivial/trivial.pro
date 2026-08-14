TEMPLATE 	= lib
TARGET      	= trivial
win32:TARGET	= nptrivial

CONFIG	       += qt dll release
LIBS	       += -lqnp
unix:LIBS      += -lXt

HEADERS		=
SOURCES		= trivial.cpp
DEF_FILE	= trivial.def
RC_FILE		= trivial.rc
