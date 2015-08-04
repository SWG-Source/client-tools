AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedDebuglinux.la
libsharedDebuglinux_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	 \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-Wno-ctor-dtor-privacy
libsharedDebuglinux_la_SOURCES=ConfigSharedDebugLinux.cpp \
	ConfigSharedDebugLinux.h \
	DebugHelp.cpp \
	DebugHelp.h \
	DebugMonitor.cpp \
	DebugMonitor.h \
	PerformanceTimer.cpp \
	PerformanceTimer.h \
	ProfilerTimer.cpp \
	ProfilerTimer.h 



include $(engine_dir)/../make_include/engine.include
