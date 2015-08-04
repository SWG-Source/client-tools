AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedFoundationlinux.la
libsharedFoundationlinux_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedIoWin/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	 \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-Wno-ctor-dtor-privacy
libsharedFoundationlinux_la_SOURCES=ByteOrder.h \
	ConfigSharedFoundation.cpp \
	ConfigSharedFoundation.h \
	FirstPlatform.h \
	FloatingPointUnit.cpp \
	FloatingPointUnit.h \
	Os.cpp \
	Os.h \
	PerThreadData.cpp \
	PerThreadData.h \
	PlatformGlue.cpp \
	PlatformGlue.h \
	SetupSharedFoundation.cpp \
	SetupSharedFoundation.h \
	vsnprintf.cpp \
	vsnprintf.h



include $(engine_dir)/../make_include/engine.include
