TEMPLATE	= app
TARGET		= httpd

CONFIG		+= qt warn_on release

REQUIRES        = network large-config

HEADERS		=
SOURCES		= httpd.cpp
