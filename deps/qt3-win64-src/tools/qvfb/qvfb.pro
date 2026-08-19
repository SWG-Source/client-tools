TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= qvfb.h qvfbview.h qvfbratedlg.h qanimationwriter.h \
		  gammaview.h skin.h
SOURCES		= qvfb.cpp qvfbview.cpp qvfbratedlg.cpp \
		  main.cpp qanimationwriter.cpp skin.cpp
INTERFACES	= config.ui
IMAGES		= images/logo.png
TARGET		= qvfb
INCLUDEPATH	+= $$QT_SOURCE_TREE/src/3rdparty/libpng $$QT_SOURCE_TREE/src/3rdparty/zlib
DEPENDPATH	= ../../include
