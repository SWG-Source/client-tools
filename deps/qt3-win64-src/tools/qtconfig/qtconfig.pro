TEMPLATE = app
CONFIG	+= qt warn_on
LANGUAGE = C++

SOURCES	+= colorbutton.cpp main.cpp previewframe.cpp previewwidget.cpp mainwindow.cpp paletteeditoradvanced.cpp
HEADERS	+= colorbutton.h previewframe.h previewwidget.h mainwindow.h paletteeditoradvanced.h
FORMS	= mainwindowbase.ui paletteeditoradvancedbase.ui previewwidgetbase.ui
IMAGES	= images/appicon.png

PROJECTNAME	= Qt Configuration
TARGET		= qtconfig
DESTDIR		= ../../bin

target.path=$$bins.path
INSTALLS	+= target
INCLUDEPATH	+= .
DBFILE		 = qtconfig.db
REQUIRES=full-config nocrosscompiler
win32:RC_FILE	= qtconfig.rc
