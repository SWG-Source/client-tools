TEMPLATE = lib
TARGET  += qpng

CONFIG += qt plugin
DESTDIR = ../../../imageformats

VERSION = 1.0.0
REQUIRES = !no-png !png

SOURCES += main.cpp

system-png {
        unix:LIBS  += -lpng
        win32:LIBS += libpng.lib
}
!system-png {
	INCLUDEPATH += $$QT_SOURCE_TREE/src/3rdparty/libpng
	INCLUDEPATH += $$QT_SOURCE_TREE/src/3rdparty/zlib
	SOURCES += \
    	    ../../../../src/3rdparty/libpng/png.c \
	    ../../../../src/3rdparty/libpng/pngerror.c \
	    ../../../../src/3rdparty/libpng/pngget.c \
	    ../../../../src/3rdparty/libpng/pngmem.c \
	    ../../../../src/3rdparty/libpng/pngpread.c \
	    ../../../../src/3rdparty/libpng/pngread.c \
	    ../../../../src/3rdparty/libpng/pngrio.c \
	    ../../../../src/3rdparty/libpng/pngrtran.c \
	    ../../../../src/3rdparty/libpng/pngrutil.c \
	    ../../../../src/3rdparty/libpng/pngset.c \
	    ../../../../src/3rdparty/libpng/pngtrans.c \
	    ../../../../src/3rdparty/libpng/pngwio.c \
	    ../../../../src/3rdparty/libpng/pngwrite.c \
	    ../../../../src/3rdparty/libpng/pngwtran.c \
	    ../../../../src/3rdparty/libpng/pngwutil.c
}

!system-zlib:SOURCES += \
	../../../../src/3rdparty/zlib/adler32.c \
	../../../../src/3rdparty/zlib/compress.c \
	../../../../src/3rdparty/zlib/crc32.c \
	../../../../src/3rdparty/zlib/deflate.c \
	../../../../src/3rdparty/zlib/gzio.c \
	../../../../src/3rdparty/zlib/inffast.c \
	../../../../src/3rdparty/zlib/inflate.c \
	../../../../src/3rdparty/zlib/inftrees.c \
	../../../../src/3rdparty/zlib/trees.c \
	../../../../src/3rdparty/zlib/uncompr.c \
	../../../../src/3rdparty/zlib/zutil.c

system-zlib:unix:LIBS += -lz
system-zlib:mac:LIBS += -lz


target.path += $$plugins.path/imageformats
INSTALLS += target
