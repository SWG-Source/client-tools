AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedFilelinux.la
libsharedFilelinux_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	 \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-Wno-ctor-dtor-privacy
libsharedFilelinux_la_SOURCES=OsFile.cpp \
	OsFile.h


include $(engine_dir)/../make_include/engine.include
