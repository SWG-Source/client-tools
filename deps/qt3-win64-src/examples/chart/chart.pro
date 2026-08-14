TEMPLATE = app

CONFIG  += warn_on

REQUIRES = full-config

HEADERS += element.h \
	   canvastext.h \
	   canvasview.h \
	   chartform.h \
	   optionsform.h \
	   setdataform.h
SOURCES += element.cpp \
	   canvasview.cpp \
	   chartform.cpp \
	   chartform_canvas.cpp \
	   chartform_files.cpp \
	   optionsform.cpp \
	   setdataform.cpp \
	   main.cpp
