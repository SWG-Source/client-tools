AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedRandomshared.la
libsharedRandomshared_la_CXXFLAGS=-I$(srcdir)/../../include/public \
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
	-Wno-ctor-dtor-privacy
libsharedRandomshared_la_SOURCES=FirstSharedRandom.h \
	ConfigSharedRandom.cpp \
	ConfigSharedRandom.h \
	Random.cpp \
	RandomGenerator.cpp \
	RandomGenerator.h \
	Random.h \
	SetupSharedRandom.cpp \
	SetupSharedRandom.h

include $(engine_dir)/../make_include/engine.include
