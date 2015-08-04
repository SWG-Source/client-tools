AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedRegexshared.la
libsharedRegexshared_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	 \
	-I$(fileInterface_dir)/include/public \
	-I$(pcre_dir)/include \
	-Wno-ctor-dtor-privacy
libsharedRegexshared_la_SOURCES=FirstSharedRegex.h \
	SetupSharedRegex.cpp \
	SetupSharedRegex.h \
	ConfigSharedRegex.cpp \
	ConfigSharedRegex.h


include $(engine_dir)/../make_include/engine.include
