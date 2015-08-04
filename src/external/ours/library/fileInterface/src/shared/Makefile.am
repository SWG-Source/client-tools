AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libfileInterfaceshared.la
libfileInterfaceshared_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(stlport_dir)/stlport \
	-Wno-ctor-dtor-privacy
libfileInterfaceshared_la_SOURCES=AbstractFile.cpp \
	AbstractFile.h \
	FirstFileInterface.h \
	StdioFile.cpp \
	StdioFile.h 
