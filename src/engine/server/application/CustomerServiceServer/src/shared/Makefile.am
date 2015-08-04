AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libCustomerServiceServershared.la
libCustomerServiceServershared_la_CXXFLAGS=-I$(srcdir)/../$(platform) \
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
	-I$(platform_dir)/include \
	-I$(platform_dir)/projects \
	-I$(soePlatform_dir)/CSAssist/projects/CSAssist/CSAssistgameapi \
	-I$(soePlatform_dir)/CSAssist/utils \
	-I$(udplibrary_dir) \
	-I$(singleton_dir)/include \
	-I$(unicodeArchive_dir)/include/public \
	-I$(localizationArchive_dir)/include/public \
	-I$(engine_dir)/server/library/serverKeyShare/include/public \
	-I$(engine_dir)/server/library/serverMetrics/include/public \
	-I$(engine_dir)/server/library/serverNetworkMessages/include/public \
	-I$(engine_dir)/server/library/serverUtility/include/public \
	-I$(libxml_dir)/include \
	-Wno-ctor-dtor-privacy
libCustomerServiceServershared_la_SOURCES=\
	CentralServerConnection.cpp \
	CentralServerConnection.h \
	ChatServerConnection.cpp \
	ChatServerConnection.h \
	ConfigCustomerServiceServer.cpp \
	ChatServerConnection.cpp \
	ChatServerConnection.h \
	ConfigCustomerServiceServer.h \
	ConnectionServerConnection.cpp \
	ConnectionServerConnection.h \
	CustomerServiceInterface.cpp \
	CustomerServiceInterface.h \
	CustomerServiceServer.cpp \
	CustomerServiceServer.h \
	CustomerServiceServerMetricsData.cpp \
	CustomerServiceServerMetricsData.h \
	FirstCustomerServiceServer.cpp \
	FirstCustomerServiceServer.h \
	GameServerConnection.cpp \
	GameServerConnection.h

include $(engine_dir)/../make_include/engine.include
