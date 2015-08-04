AUTOMAKE_OPTIONS=foreign
SUBDIRS=UdpLibraryMT
noinst_LTLIBRARIES=libsharedNetworkshared.la
libsharedNetworkshared_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(srcdir)/../../include/private \
	-I$(srcdir)/../../src/$(platform) \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedLog/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMessageDispatch/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	 \
	-I$(udplibrary_dir) \
	-I$(archive_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy
libsharedNetworkshared_la_SOURCES=Address.h \
	BroadcastSock.cpp \
	BroadcastSock.h \
	ConfigSharedNetwork.cpp \
	ConfigSharedNetwork.h \
	Connection.cpp \
	Connection.h \
	ConnectionHandler.cpp \
	ConnectionHandler.h \
	FirstSharedNetwork.h \
	ManagerHandler.cpp \
	ManagerHandler.h \
	NetworkHandler.cpp \
	NetworkHandler.h \
	NetworkSetupData.cpp \
	NetworkSetupData.h \
	Service.cpp \
	Service.h \
	SetupSharedNetwork.cpp \
	SetupSharedNetwork.h \
	UdpSock.h

include $(engine_dir)/../make_include/engine.include
