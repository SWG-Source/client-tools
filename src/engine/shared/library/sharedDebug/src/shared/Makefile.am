AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedDebugshared.la
libsharedDebugshared_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	 \
	-Wno-ctor-dtor-privacy
libsharedDebugshared_la_SOURCES=CallStack.cpp \
	CallStackCollector.cpp \
	DataLint.cpp \
	DebugFlags.cpp \
	DebugKey.cpp \
	InstallTimer.cpp \
	PixCounter.cpp \
	Profiler.cpp \
	RemoteDebug.cpp \
	RemoteDebug_inner.cpp \
	Report.cpp \
	SetupSharedDebug.cpp \
	CallStack.h \
	CallStackCollector.h \
	DataLint.h \
	DebugFlags.h \
	DebugKey.h \
	FirstSharedDebug.h \
	InstallTimer.h \
	PixCounter.h \
	Profiler.h \
	RemoteDebug.h \
	RemoteDebug_inner.h \
	Report.h \
	SetupSharedDebug.h



include $(engine_dir)/../make_include/engine.include
