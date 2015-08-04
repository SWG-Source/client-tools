AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libCommoditiesServershared.la
libCommoditiesServershared_la_CXXFLAGS=-I$(srcdir)/../$(platform) \
	-I$(archive_dir)/include \
	-I$(unicode_dir)/include \
	-I$(localization_dir)/include \
	-I$(engine_dir)/shared/library/sharedCommandParser/include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDatabaseInterface/include/public \
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
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(engine_dir)/shared/library/sharedUtility/include/public \
	-I$(stlport_dir)/stlport \
	-I$(perforce_dir)/include \
	-I$(singleton_dir)/include \
	-I$(unicodeArchive_dir)/include/public \
	-I$(localizationArchive_dir)/include/public \
	-I$(engine_dir)/server/library/serverKeyShare/include/public \
	-I$(engine_dir)/server/library/serverMetrics/include/public \
	-I$(engine_dir)/server/library/serverNetworkMessages/include/public \
	-I$(engine_dir)/server/library/serverUtility/include/public \
	-Wno-ctor-dtor-privacy

libCommoditiesServershared_la_SOURCES=AuctionBid.cpp \
	AuctionBid.h \
	Auction.cpp \
	Auction.h \
	AuctionItem.cpp \
	AuctionItem.h \
	AuctionLocation.cpp \
	AuctionLocation.h \
	AuctionMarket.cpp \
	AuctionMarket.h \
	CentralServerConnection.cpp \
	CentralServerConnection.h \
	CommodityServer.cpp \
	CommodityServer.h \
	CommodityServerMetricsData.cpp \
	CommodityServerMetricsData.h \
	ConfigCommodityServer.cpp \
	ConfigCommodityServer.h \
	DatabaseServerConnection.cpp \
	DatabaseServerConnection.h \
	dBAuctionRecord.h \
	dBBidRecord.h \
	dBCharacterRecord.h \
	dBLocationRecord.h \
	FirstCommodityServer.h \
	GameServerConnection.cpp \
	GameServerConnection.h

include $(engine_dir)/../make_include/engine.include
