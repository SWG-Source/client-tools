AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedIoWinshared.la
libsharedIoWinshared_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	 \
	-I$(archive_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy
libsharedIoWinshared_la_SOURCES=FirstSharedIoWin.h \
	ConfigSharedIoWin.cpp \
	ConfigSharedIoWin.h \
	IoWin.cpp \
	IoWin.def \
	IoWin.h \
	IoWinManager.cpp \
	IoWinManager.h \
	SetupSharedIoWin.cpp \
	SetupSharedIoWin.h

include $(engine_dir)/../make_include/engine.include
