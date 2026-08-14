TEMPLATE = lib
TARGET  += qjpeg

CONFIG  += qt plugin
DESTDIR  = ../../../imageformats

VERSION = 1.0.0
REQUIRES = !no-jpeg !jpeg

SOURCES += main.cpp
system-jpeg {
        unix:LIBS += -ljpeg
        win32:LIBS += libjpeg.lib
}
!system-jpeg {
	INCLUDEPATH += ../../../../src/3rdparty/libjpeg
	SOURCES  += \
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
