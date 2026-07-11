TEMPLATE = lib
TARGET	 = qsqlibase

CONFIG	+= qt plugin
DESTDIR	 = ../../../sqldrivers

HEADERS		= ../../../../src/sql/drivers/ibase/qsql_ibase.h
SOURCES		= main.cpp \
		  ../../../../src/sql/drivers/ibase/qsql_ibase.cpp

unix {
	OBJECTS_DIR = .obj

	!contains( LIBS, .*gds.* ):!contains( LIBS, .*libfb.* ) {
	    LIBS    *= -lgds
	}
}
win32 {
	OBJECTS_DIR = obj
	!win32-borland:LIBS *= gds32_ms.lib
	win32-borland:LIBS  += gds32.lib
}

REQUIRES	= sql

target.path += $$plugins.path/sqldrivers
INSTALLS += target
