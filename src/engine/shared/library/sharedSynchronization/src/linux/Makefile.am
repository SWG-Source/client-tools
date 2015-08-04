AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedSynchronizationlinux.la
libsharedSynchronizationlinux_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	 \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-Wno-ctor-dtor-privacy
libsharedSynchronizationlinux_la_SOURCES=ConditionVariable.cpp \
	ConditionVariable.h \
	Gate.cpp \
	Gate.h \
	InterlockedInteger.h \
	InterlockedVoidPointer.h \
	Mutex.cpp \
	Mutex.h \
	RecursiveMutex.cpp \
	RecursiveMutex.h \
	Semaphore.cpp \
	Semaphore.h

include $(engine_dir)/../make_include/engine.include
