AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libunicodeArchiveshared.la
libunicodeArchiveshared_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(stlport_dir)/stlport \
	-I$(archive_dir)/include \
	-I$(unicode_dir)/include \
	-Wno-ctor-dtor-privacy
libunicodeArchiveshared_la_SOURCES=FirstUnicodeArchive.h \
	UnicodeArchive.cpp \
	UnicodeArchive.h \
	UnicodeAutoDeltaPackedMap.cpp \
	UnicodeAutoDeltaPackedMap.h
