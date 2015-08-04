AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedRegexlinux.la
libsharedRegexlinux_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	 \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-Wno-ctor-dtor-privacy
libsharedRegexlinux_la_SOURCES=RegexServices.cpp \
	RegexServices.h



include $(engine_dir)/../make_include/engine.include
