AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libswgServerNetworkMessagessharedresource.la
libswgServerNetworkMessagessharedresource_la_CXXFLAGS=-I$(srcdir)/../../../include/public \
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
	-I$(engine_dir)/shared/library/sharedNetwork/include/public \
	-I$(engine_dir)/shared/library/sharedNetworkMessages/include/public \
	-I$(engine_dir)/shared/library/sharedObject/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	 \
	-I$(udplibrary_dir) \
	-I$(singleton_dir)/include \
	-I$(archive_dir)/include \
	-I$(unicode_dir)/include \
	-I$(localization_dir)/include \
	-I$(unicodeArchive_dir)/include/public \
	-I$(localizationArchive_dir)/include/public \
	-I$(fileInterface_dir)/include/public \
	-I$(game_dir)/shared/library/swgSharedUtility/include/public \
	-Wno-ctor-dtor-privacy

libswgServerNetworkMessagessharedresource_la_SOURCES=MessageQueueActivateInstallation.cpp \
	MessageQueueActivateInstallation.h \
	MessageQueueDeactivateInstallation.cpp \
	MessageQueueDeactivateInstallation.h \
	MessageQueueInstallationHarvest.cpp \
	MessageQueueInstallationHarvest.h \
	MessageQueueResourceAdd.cpp \
	MessageQueueResourceAdd.h \
	MessageQueueResourceFinishHarvest.cpp \
	MessageQueueResourceFinishHarvest.h \
	MessageQueueResourceSetInstalledEfficiency.cpp \
	MessageQueueResourceSetInstalledEfficiency.h \
	MessageQueueResourceTypeDepleted.cpp \
	MessageQueueResourceTypeDepleted.h
