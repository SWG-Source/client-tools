AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libcryptosharedoriginal.la
libcryptosharedoriginal_la_CXXFLAGS=-I$(crypto_dir)/include \
	-I$(stlport_dir)/stlport \
	-Wno-ctor-dtor-privacy
CXXFLAGS=-g -O2
libcryptosharedoriginal_la_SOURCES=config.h \
	cryptlib.cpp \
	cryptlib.h \
	filters.cpp \
	filters.h \
	iterhash.cpp \
	iterhash.h \
	md5.cpp \
	md5.h \
	misc.cpp \
	misc.h \
	mqueue.cpp \
	mqueue.h \
	queue.cpp \
	queue.h \
	smartptr.h \
	tftables.cpp \
	twofish.cpp \
	twofish.h \
	words.h
