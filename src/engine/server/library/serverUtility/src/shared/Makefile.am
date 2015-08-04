AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libserverUtilityshared.la
libserverUtilityshared_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(srcdir)/../../include/private \
	-I$(srcdir)/../../src/$(platform) \
	-I$(engine_dir)/server/library/serverUtility/include/public \
	-I$(engine_dir)/server/library/serverNetworkMessages/include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedLog/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMathArchive/include/public \
	-I$(engine_dir)/shared/library/sharedMessageDispatch/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedNetwork/include/public \
	-I$(engine_dir)/shared/library/sharedNetworkMessages/include/public \
	-I$(engine_dir)/shared/library/sharedRandom/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(engine_dir)/shared/library/sharedUtility/include/public \
	-I$(stlport_dir)/stlport \
	-I$(singleton_dir)/include \
	-I$(udplibrary_dir) \
	-I$(archive_dir)/include \
	-I$(crypto_dir)/include \
	-I$(localization_dir)/include \
	-I$(localization_dir)/include \
	-I$(localizationArchive_dir)/include/public \
	-I$(fileInterface_dir)/include/public \
	-I$(unicode_dir)/include \
	-I$(unicodeArchive_dir)/include/public \
	-Wno-ctor-dtor-privacy
libserverUtilityshared_la_SOURCES=AdminAccountManager.cpp \
	AdminAccountManager.h \
	ChatLogManager.cpp \
	ConfigServerUtility.cpp \
	ConfigServerUtility.h \
	ClusterWideDataManager.cpp \
	ClusterWideDataManager.h \
	ClusterWideDataManagerList.cpp \
	ClusterWideDataManagerList.h \
	FirstServerUtility.h \
	FreeCtsDataTable.cpp \
	FreeCtsDataTable.h \
	LocationData.cpp \
	LocationData.h \
	MissionLocation.cpp \
	MissionLocation.h \
	PopulationList.cpp \
	PopulationList.h \
	PvpEnemy.cpp \
	PvpEnemy.h \
	ResourceFractalData.cpp \
	ResourceFractalData.h \
	RecentMaxSyncedValue.cpp \
	RecentMaxSyncedValue.h \
	ServerClock.cpp \
	ServerClock.h \
	ServerConnection.cpp \
	ServerConnection.h \
	ServerServiceHandler.h \
	SetupServerUtility.cpp \
	SetupServerUtility.h \
	SystemAssignedProcessId.cpp \
	SystemAssignedProcessId.h



include $(engine_dir)/../make_include/engine.include
