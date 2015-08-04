AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedNetworkUdpLibraryMT.la
libsharedNetworkUdpLibraryMT_la_CXXFLAGS=-I$(srcdir)/../../../include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedLog/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	 \
	-I$(udplibrary_dir) \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy
libsharedNetworkUdpLibraryMT_la_SOURCES=Events.cpp \
	Events.h \
	UdpConnectionHandlerMT.cpp \
	UdpConnectionHandlerMT.h \
	UdpConnectionMT.cpp \
	UdpConnectionMT.h \
	UdpHandlerMT.h \
	UdpLibraryMT.cpp \
	UdpLibraryMT.h \
	UdpManagerHandlerMT.cpp \
	UdpManagerHandlerMT.h \
	UdpManagerMT.cpp \
	UdpManagerMT.h \
	UdpMiscMT.cpp \
	UdpMiscMT.h

include $(engine_dir)/../make_include/engine.include
