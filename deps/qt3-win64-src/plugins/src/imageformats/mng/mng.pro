TEMPLATE = lib
TARGET  += qmng

CONFIG  += qt plugin
DESTDIR  = ../../../imageformats

VERSION = 1.0.0
REQUIRES = !no-mng !mng

SOURCES += main.cpp

win32-borland {
	QMAKE_CFLAGS_WARN_ON	+= -w-par
	QMAKE_CXXFLAGS_WARN_ON	+= -w-par
}
win32: CONFIG-=zlib system-zlib jpeg system-jpeg

system-mng {
        win32:LIBS += libmng.lib
        unix:LIBS  += -lmng
}
!system-mng {
	INCLUDEPATH += ../../../../src/3rdparty/libmng
	SOURCES += \
	../../../../src/3rdparty/libmng/libmng_callback_xs.c \
	../../../../src/3rdparty/libmng/libmng_chunk_io.c \
	../../../../src/3rdparty/libmng/libmng_chunk_prc.c \
	../../../../src/3rdparty/libmng/libmng_chunk_xs.c \
	../../../../src/3rdparty/libmng/libmng_cms.c \
	../../../../src/3rdparty/libmng/libmng_display.c \
	../../../../src/3rdparty/libmng/libmng_dither.c \
	../../../../src/3rdparty/libmng/libmng_error.c \
	../../../../src/3rdparty/libmng/libmng_filter.c \
	../../../../src/3rdparty/libmng/libmng_hlapi.c \
	../../../../src/3rdparty/libmng/libmng_jpeg.c \
	../../../../src/3rdparty/libmng/libmng_object_prc.c \
	../../../../src/3rdparty/libmng/libmng_pixels.c \
	../../../../src/3rdparty/libmng/libmng_prop_xs.c \
	../../../../src/3rdparty/libmng/libmng_read.c \
	../../../../src/3rdparty/libmng/libmng_trace.c \
	../../../../src/3rdparty/libmng/libmng_write.c \
	../../../../src/3rdparty/libmng/libmng_zlib.c

	HEADERS += ../../../../src/3rdparty/libmng/libmng.h \
	../../../../src/3rdparty/libmng/libmng_chunks.h \
	../../../../src/3rdparty/libmng/libmng_chunk_io.h \
	../../../../src/3rdparty/libmng/libmng_chunk_prc.h \
	../../../../src/3rdparty/libmng/libmng_cms.h \
	../../../../src/3rdparty/libmng/libmng_conf.h \
	../../../../src/3rdparty/libmng/libmng_data.h \
	../../../../src/3rdparty/libmng/libmng_display.h \
	../../../../src/3rdparty/libmng/libmng_dither.h \
	../../../../src/3rdparty/libmng/libmng_error.h \
	../../../../src/3rdparty/libmng/libmng_filter.h \
	../../../../src/3rdparty/libmng/libmng_jpeg.h \
	../../../../src/3rdparty/libmng/libmng_memory.h \
	../../../../src/3rdparty/libmng/libmng_objects.h \
	../../../../src/3rdparty/libmng/libmng_object_prc.h \
	../../../../src/3rdparty/libmng/libmng_pixels.h \
	../../../../src/3rdparty/libmng/libmng_read.h \
	../../../../src/3rdparty/libmng/libmng_trace.h \
	../../../../src/3rdparty/libmng/libmng_types.h \
	../../../../src/3rdparty/libmng/libmng_write.h \
	../../../../src/3rdparty/libmng/libmng_zlib.h
}

!system-zlib {
	INCLUDEPATH += ../../../../src/3rdparty/zlib
	SOURCES+= \
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
}
!no-zlib:!zlib:unix:LIBS += -lz
!no-zlib:!zlib:mac:LIBS += -lz

system-jpeg {
        unix:LIBS  += -ljpeg
        win32:LIBS += libjpeg.lib
}
!system-jpeg {
	INCLUDEPATH += ../../../../src/3rdparty/libjpeg
	SOURCES += \
	    ../../../../src/3rdparty/libjpeg/jcapimin.c \
	    ../../../../src/3rdparty/libjpeg/jcapistd.c \
	    ../../../../src/3rdparty/libjpeg/jccoefct.c \
	    ../../../../src/3rdparty/libjpeg/jccolor.c \
	    ../../../../src/3rdparty/libjpeg/jcdctmgr.c \
	    ../../../../src/3rdparty/libjpeg/jchuff.c \
	    ../../../../src/3rdparty/libjpeg/jcinit.c \
	    ../../../../src/3rdparty/libjpeg/jcmainct.c \
	    ../../../../src/3rdparty/libjpeg/jcmarker.c \
	    ../../../../src/3rdparty/libjpeg/jcmaster.c \
	    ../../../../src/3rdparty/libjpeg/jcomapi.c \
	    ../../../../src/3rdparty/libjpeg/jcparam.c \
	    ../../../../src/3rdparty/libjpeg/jcphuff.c \
	    ../../../../src/3rdparty/libjpeg/jcprepct.c \
	    ../../../../src/3rdparty/libjpeg/jcsample.c \
	    ../../../../src/3rdparty/libjpeg/jctrans.c \
	    ../../../../src/3rdparty/libjpeg/jdapimin.c \
	    ../../../../src/3rdparty/libjpeg/jdapistd.c \
	    ../../../../src/3rdparty/libjpeg/jdatadst.c \
	    ../../../../src/3rdparty/libjpeg/jdatasrc.c \
	    ../../../../src/3rdparty/libjpeg/jdcoefct.c \
	    ../../../../src/3rdparty/libjpeg/jdcolor.c \
	    ../../../../src/3rdparty/libjpeg/jddctmgr.c \
	    ../../../../src/3rdparty/libjpeg/jdhuff.c \
	    ../../../../src/3rdparty/libjpeg/jdinput.c \
	    ../../../../src/3rdparty/libjpeg/jdmainct.c \
	    ../../../../src/3rdparty/libjpeg/jdmarker.c \
	    ../../../../src/3rdparty/libjpeg/jdmaster.c \
	    ../../../../src/3rdparty/libjpeg/jdmerge.c \
	    ../../../../src/3rdparty/libjpeg/jdphuff.c \
	    ../../../../src/3rdparty/libjpeg/jdpostct.c \
	    ../../../../src/3rdparty/libjpeg/jdsample.c \
	    ../../../../src/3rdparty/libjpeg/jdtrans.c \
	    ../../../../src/3rdparty/libjpeg/jerror.c \
	    ../../../../src/3rdparty/libjpeg/jfdctflt.c \
	    ../../../../src/3rdparty/libjpeg/jfdctfst.c \
	    ../../../../src/3rdparty/libjpeg/jfdctint.c \
	    ../../../../src/3rdparty/libjpeg/jidctflt.c \
	    ../../../../src/3rdparty/libjpeg/jidctfst.c \
	    ../../../../src/3rdparty/libjpeg/jidctint.c \
	    ../../../../src/3rdparty/libjpeg/jidctred.c \
	    ../../../../src/3rdparty/libjpeg/jmemmgr.c \
	    ../../../../src/3rdparty/libjpeg/jquant1.c \
	    ../../../../src/3rdparty/libjpeg/jquant2.c \
	    ../../../../src/3rdparty/libjpeg/jutils.c \
	    ../../../../src/3rdparty/libjpeg/jmemnobs.c
}


target.path += $$plugins.path/imageformats
INSTALLS += target
