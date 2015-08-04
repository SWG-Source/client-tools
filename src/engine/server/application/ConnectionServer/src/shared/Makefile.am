AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libConnectionServershared.la
libConnectionServershared_la_CXXFLAGS=-I$(srcdir)/../$(platform) \
	-I$(archive_dir)/include \
	-I$(unicode_dir)/include \
	-I$(localization_dir)/include \
	-I$(engine_dir)/shared/library/sharedCommandParser/include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedGame/include/public \
	-I$(engine_dir)/shared/library/sharedLog/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMathArchive/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedMessageDispatch/include/public \
	-I$(engine_dir)/shared/library/sharedNetwork/include/public \
	-I$(engine_dir)/shared/library/sharedNetworkMessages/include/public \
	-I$(engine_dir)/shared/library/sharedObject/include/public \
	-I$(engine_dir)/shared/library/sharedRandom/include/public \
	-I$(engine_dir)/shared/library/sharedRegex/include/public \
	-I$(engine_dir)/shared/library/sharedTemplate/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(engine_dir)/shared/library/sharedUtility/include/public \
	-I$(stlport_dir)/stlport \
	-I$(perforce_dir)/include \
	-I$(soePlatform_dir)/ChatAPI2/ChatAPI/utils/ \
	-I$(soePlatform_dir)/ChatAPI2/ChatAPI/projects/Chat \
	-I$(platform_dir)/include \
	-I$(platform_dir)/projects \
	-I$(udplibrary_dir) \
	-I$(singleton_dir)/include \
	-I$(unicodeArchive_dir)/include/public \
	-I$(localizationArchive_dir)/include/public \
	-I$(engine_dir)/server/library/serverKeyShare/include/public \
	-I$(engine_dir)/server/library/serverMetrics/include/public \
	-I$(engine_dir)/server/library/serverNetworkMessages/include/public \
	-I$(engine_dir)/server/library/serverUtility/include/public \
	-Wno-ctor-dtor-privacy
libConnectionServershared_la_SOURCES = CentralConnection.cpp 	CentralConnection.h 	ChatServerConnection.cpp 	ChatServerConnection.h 	ClientConnection.cpp 	ClientConnection.h 	Client.cpp 	Client.h 	ConfigConnectionServer.cpp 	ConfigConnectionServer.h 	ConnectionServer.cpp 	ConnectionServer.h 	ConnectionServerMetricsData.cpp 	ConnectionServerMetricsData.h 	CustomerServiceConnection.cpp 	CustomerServiceConnection.h 	FirstConnectionServer.h 	GameConnection.cpp 	GameConnection.h 	SessionApiClient.cpp 	SessionApiClient.h PseudoClientConnection.cpp 
noinst_HEADERS =  PseudoClientConnection.h

include $(engine_dir)/../make_include/engine.include
