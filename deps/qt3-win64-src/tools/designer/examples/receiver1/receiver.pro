TEMPLATE = app
LANGUAGE = C++
TARGET   = receiver

CONFIG	+= qt warn_on release
INCLUDEPATH	+= $$QT_SOURCE_TREE/tools/designer/uilib
LIBS	+= -lqui
SOURCES	+= main.cpp
IMAGEFILE	= images.cpp
FORMS	= mainform.ui
DBFILE	= receiver.db
