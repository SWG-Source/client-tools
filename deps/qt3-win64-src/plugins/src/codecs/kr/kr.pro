TEMPLATE = lib
TARGET	 = qkrcodecs

CONFIG	+= qt warn_on plugin
DESTDIR	 = ../../../codecs

REQUIRES = !bigcodecs

HEADERS		= ../../../../include/qeuckrcodec.h \
		  ../../../../include/private/qfontcodecs_p.h

SOURCES		= ../../../../src/codecs/qeuckrcodec.cpp \
		  ../../../../src/codecs/qfontkrcodec.cpp \
		  main.cpp


target.path += $$plugins.path/codecs
INSTALLS += target

