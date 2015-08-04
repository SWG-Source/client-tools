AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedThreadlinux.la
libsharedThreadlinux_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	 \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-Wno-ctor-dtor-privacy
libsharedThreadlinux_la_SOURCES=Thread.cpp \
	Thread.h



include $(engine_dir)/../make_include/engine.include
