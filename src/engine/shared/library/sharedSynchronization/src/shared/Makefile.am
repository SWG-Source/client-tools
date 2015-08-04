AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedSynchronizationshared.la
libsharedSynchronizationshared_la_CXXFLAGS=-I$(srcdir)/../../include/public \
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
libsharedSynchronizationshared_la_SOURCES=BlockingPointer.h \
	BlockingQueue.h \
	CountingSemaphore.h \
	FirstSharedSynchronization.h \
	Guard.h \
	WriteOnce.h

include $(engine_dir)/../make_include/engine.include
