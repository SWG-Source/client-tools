AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedThreadshared.la
libsharedThreadshared_la_CXXFLAGS=-I$(srcdir)/../../include/public \
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
libsharedThreadshared_la_SOURCES=RunThread.cpp \
	RunThread.h \
	SetupSharedThread.cpp \
	SetupSharedThread.h \
	ThreadHandle.h

include $(engine_dir)/../make_include/engine.include
