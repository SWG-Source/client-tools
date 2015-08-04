AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=liblocalizationArchiveshared.la
liblocalizationArchiveshared_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(stlport_dir)/stlport \
	-I$(localization_dir)/include \
	-I$(unicode_dir)/include \
	-I$(archive_dir)/include \
	-Wno-ctor-dtor-privacy
liblocalizationArchiveshared_la_SOURCES=FirstLocalizationArchive.h \
	StringIdArchive.cpp \
	StringIdArchive.h
