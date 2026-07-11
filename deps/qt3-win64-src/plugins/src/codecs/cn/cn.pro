TEMPLATE = lib
TARGET	 = qcncodecs

CONFIG	+= qt warn_on plugin
DESTDIR	 = ../../../codecs

REQUIRES = !bigcodecs

HEADERS		= ../../../../include/qgb18030codec.h \
		  ../../../../include/private/qfontcodecs_p.h

SOURCES		= ../../../../src/codecs/qgb18030codec.cpp \
		  ../../../../src/codecs/qfontcncodec.cpp \
		  main.cpp


target.path += $$plugins.path/codecs
INSTALLS += target
