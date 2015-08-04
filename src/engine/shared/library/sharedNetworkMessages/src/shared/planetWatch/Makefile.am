AUTOMAKE_OPTIONS=foreign

lib_LTLIBRARIES=libsharedNetworkMessagessharedplanetWatch.la
libsharedNetworkMessagessharedplanetWatch_la_CXXFLAGS=-I$(srcdir)/../../../include/public \
	-I$(srcdir)/../../../include/private \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedLog/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMathArchive/include/public \
	-I$(engine_dir)/shared/library/sharedMessageDispatch/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	 \
	-I$(udplibrary_dir) \
	-I$(archive_dir)/include \
	-I$(unicode_dir)/include \
	-I$(localization_dir)/include \
	-I$(unicodeArchive_dir)/include/public \
	-I$(localizationArchive_dir)/include/public \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy

libsharedNetworkMessagessharedplanetWatch_la_SOURCES=GameServerStatus.cpp \
	GameServerStatus.h \
	PlanetNodeStatusMessage.cpp \
	PlanetNodeStatusMessage.h \
	PlanetObjectStatusMessage.cpp \
	PlanetObjectStatusMessage.h \
	ServerInfoArchive.cpp \
	ServerInfoArchive.h \
	ServerInfo.h

libsharedNetworkMessagessharedplanetWatch_la_LDFLAGS=-avoid-version


include $(engine_dir)/../make_include/engine.include
