TEMPLATE = lib
TARGET	 = qsqlodbc

CONFIG  += qt plugin
DESTDIR	 = ../../../sqldrivers

HEADERS		= ../../../../src/sql/drivers/odbc/qsql_odbc.h
SOURCES		= main.cpp \
		  ../../../../src/sql/drivers/odbc/qsql_odbc.cpp

mac {
        !contains( LIBS, .*odbc.* ) {
            LIBS        *= -liodbc
        }
}

unix {
	OBJECTS_DIR	= .obj
	!contains( LIBS, .*odbc.* ) {
	    LIBS 	*= -lodbc
	}
}

win32 {
	OBJECTS_DIR		= obj
	!win32-borland:LIBS	*= -lodbc32
    	win32-borland:LIBS	*= $(BCB)/lib/PSDK/odbc32.lib
}

REQUIRES	= sql

target.path += $$plugins.path/sqldrivers
INSTALLS += target
