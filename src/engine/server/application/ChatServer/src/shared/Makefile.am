AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libChatServershared.la
libChatServershared_la_CXXFLAGS=-I$(srcdir)/../$(platform) \
	-I$(archive_dir)/include \
	-I$(unicode_dir)/include \
	-I$(localization_dir)/include \
	-I$(engine_dir)/shared/library/sharedCommandParser/include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
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
	-I$(soePlatform_dir)/VChatAPI/utils2.0/utils \
	-I$(soePlatform_dir)/VChatAPI/projects/VChat \
	-I$(soePlatform_dir)/ChatAPI2/ChatAPI/utils/ \
	-I$(soePlatform_dir)/ChatAPI2/ChatAPI/projects/Chat \
	-I$(platform_dir)/include \
	-I$(singleton_dir)/include \
	-I$(unicodeArchive_dir)/include/public \
	-I$(localizationArchive_dir)/include/public \
	-I$(engine_dir)/server/library/serverKeyShare/include/public \
	-I$(engine_dir)/server/library/serverMetrics/include/public \
	-I$(engine_dir)/server/library/serverNetworkMessages/include/public \
	-I$(engine_dir)/server/library/serverUtility/include/public \
	-Wno-ctor-dtor-privacy \
	-DEXTERNAL_DISTRO -DNAMESPACE="ChatSystem" -D_REENTRANT
	
libChatServershared_la_SOURCES = CentralServerConnection.cpp CentralServerConnection.h ChatInterface.cpp ChatInterface.h ChatServerAvatarOwner.cpp ChatServerAvatarOwner.h ChatServer.cpp ChatServer.h ChatServerRoomOwner.cpp ChatServerRoomOwner.h ConfigChatServer.cpp ConfigChatServer.h ConnectionServerConnection.cpp ConnectionServerConnection.h CustomerServiceServerConnection.cpp CustomerServiceServerConnection.h FirstChatServer.cpp FirstChatServer.h GameServerConnection.cpp GameServerConnection.h PlanetServerConnection.h PlanetServerServerConnection.cpp ChatServerMetricsData.cpp VChatInterface.h VChatInterface.cpp

noinst_HEADERS =  ChatServerMetricsData.h

include $(engine_dir)/../make_include/engine.include
