AUTOMAKE_OPTIONS=foreign

lib_LTLIBRARIES=libsharedNetworkMessagessharedcommon.la
libsharedNetworkMessagessharedcommon_la_CXXFLAGS=-I$(srcdir)/../../../include/public \
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
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(engine_dir)/shared/library/sharedUtility/include/public \
	 \
	-I$(udplibrary_dir) \
	-I$(singleton_dir)/include \
	-I$(archive_dir)/include \
	-I$(unicode_dir)/include \
	-I$(localization_dir)/include \
	-I$(unicodeArchive_dir)/include/public \
	-I$(localizationArchive_dir)/include/public \
	-I$(fileInterface_dir)/include/public \
	-I$(singleton_dir) \
	-Wno-ctor-dtor-privacy

libsharedNetworkMessagessharedcommon_la_SOURCES=BaselinesMessage.cpp \
	BaselinesMessage.h \
	BatchBaselinesMessage.cpp \
	BatchBaselinesMessage.h \
	ClientNotificationBoxMessage.cpp \
	ClientNotificationBoxMessage.h \
	ClusterWideDataGetElementMessage.cpp \
	ClusterWideDataGetElementMessage.h \
	ClusterWideDataGetElementResponseMessage.cpp \
	ClusterWideDataGetElementResponseMessage.h \
	ClusterWideDataReleaseLockMessage.cpp \
	ClusterWideDataReleaseLockMessage.h \
	ClusterWideDataRemoveElementMessage.cpp \
	ClusterWideDataRemoveElementMessage.h \
	ClusterWideDataUpdateDictionaryMessage.cpp \
	ClusterWideDataUpdateDictionaryMessage.h \
	DeltasMessage.cpp \
	DeltasMessage.h \
	ErrorMessage.cpp \
	ErrorMessage.h \
	FrameEndMessage.cpp \
	FrameEndMessage.h \
	GameNetworkMessage.cpp \
	GameNetworkMessage.h \
	GenericValueTypeMessage.h \
	MessageQueueGenericValueType.h \
	MessageQueueNetworkIdAndTransform.cpp \
	MessageQueueNetworkIdAndTransform.h \
	MessageQueueUpdateShipOnCollision.cpp \
	MessageQueueUpdateShipOnCollision.h \
	SetTransformMessage.cpp \
	SetTransformMessage.h

libsharedNetworkMessagessharedcommon_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
