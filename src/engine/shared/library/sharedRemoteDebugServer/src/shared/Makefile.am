AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedRemoteDebugServershared.la
libsharedRemoteDebugServershared_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedNetwork/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	 \
	-I$(archive_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy
libsharedRemoteDebugServershared_la_SOURCES=FirstSharedRemoteDebugServer.cpp \
	FirstSharedRemoteDebugServer.h \
	SharedRemoteDebugServerConnection.cpp \
	SharedRemoteDebugServerConnection.h \
	SharedRemoteDebugServer.cpp \
	SharedRemoteDebugServer.h

include $(engine_dir)/../make_include/engine.include
