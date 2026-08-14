TEMPLATE = lib
TARGET	 = qsqlite

CONFIG	+= qt plugin
DESTDIR	 = ../../../sqldrivers

HEADERS		= ../../../../src/sql/drivers/sqlite/qsql_sqlite.h
SOURCES		= smain.cpp \
		  ../../../../src/sql/drivers/sqlite/qsql_sqlite.cpp

unix {
	OBJECTS_DIR = .obj
}

win32 {
	OBJECTS_DIR = obj
#	win32-msvc: {
#		LIBS *= delayimp.lib
#		QMAKE_LFLAGS += /DELAYLOAD:libsqlite.dll
#	}
#	win32-borland: {
#		QMAKE_LFLAGS += /dlibsqlite.dll
#	}
}

!contains( LIBS, .*sqlite.* ) {
    INCLUDEPATH += ../../../../src/3rdparty/sqlite

    HEADERS += ../../../../src/3rdparty/sqlite/btree.h \
               ../../../../src/3rdparty/sqlite/config.h \
               ../../../../src/3rdparty/sqlite/hash.h \
               ../../../../src/3rdparty/sqlite/opcodes.h \
               ../../../../src/3rdparty/sqlite/os.h \
               ../../../../src/3rdparty/sqlite/pager.h \
               ../../../../src/3rdparty/sqlite/parse.h \
               ../../../../src/3rdparty/sqlite/sqlite.h \
               ../../../../src/3rdparty/sqlite/sqliteInt.h \
               ../../../../src/3rdparty/sqlite/vdbe.h \
	       ../../../../src/3rdparty/sqlite/vdbeInt.h

    SOURCES += ../../../../src/3rdparty/sqlite/attach.c \
               ../../../../src/3rdparty/sqlite/auth.c \
               ../../../../src/3rdparty/sqlite/btree.c \
               ../../../../src/3rdparty/sqlite/btree_rb.c \
               ../../../../src/3rdparty/sqlite/build.c \
               ../../../../src/3rdparty/sqlite/copy.c \
	       ../../../../src/3rdparty/sqlite/date.c \
               ../../../../src/3rdparty/sqlite/delete.c \
               ../../../../src/3rdparty/sqlite/expr.c \
               ../../../../src/3rdparty/sqlite/func.c \
               ../../../../src/3rdparty/sqlite/hash.c \
               ../../../../src/3rdparty/sqlite/insert.c \
               ../../../../src/3rdparty/sqlite/main.c \
               ../../../../src/3rdparty/sqlite/opcodes.c \
               ../../../../src/3rdparty/sqlite/os.c \
               ../../../../src/3rdparty/sqlite/pager.c \
               ../../../../src/3rdparty/sqlite/parse.c \
               ../../../../src/3rdparty/sqlite/pragma.c \
               ../../../../src/3rdparty/sqlite/printf.c \
               ../../../../src/3rdparty/sqlite/random.c \
               ../../../../src/3rdparty/sqlite/select.c \
               ../../../../src/3rdparty/sqlite/shell.c \
               ../../../../src/3rdparty/sqlite/table.c \
               ../../../../src/3rdparty/sqlite/tokenize.c \
               ../../../../src/3rdparty/sqlite/trigger.c \
               ../../../../src/3rdparty/sqlite/update.c \
               ../../../../src/3rdparty/sqlite/util.c \
               ../../../../src/3rdparty/sqlite/vacuum.c \
               ../../../../src/3rdparty/sqlite/vdbe.c \
	       ../../../../src/3rdparty/sqlite/vdbeaux.c \
               ../../../../src/3rdparty/sqlite/where.c
}

REQUIRES	= sql

target.path += $$plugins.path/sqldrivers
INSTALLS += target
