AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libserverNetworkMessagessharedcore.la

libserverNetworkMessagessharedcore_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(srcdir)/../../include/private \
	-I$(srcdir)/../../src/$(platform) \
	-I$(engine_dir)/server/library/serverGame/include/public \
	-I$(engine_dir)/server/library/serverUtility/include/public \
	-I$(engine_dir)/server/library/serverNetworkMessages/include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedGame/include/public \
	-I$(engine_dir)/shared/library/sharedLog/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMathArchive/include/public \
	-I$(engine_dir)/shared/library/sharedMessageDispatch/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedNetwork/include/public \
	-I$(engine_dir)/shared/library/sharedNetworkMessages/include/public \
	-I$(engine_dir)/shared/library/sharedObject/include/public \
	-I$(engine_dir)/shared/library/sharedRandom/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(engine_dir)/shared/library/sharedUtility/include/public \
	-I$(stlport_dir)/stlport \
	-I$(singleton_dir)/include \
	-I$(udplibrary_dir) \
	-I$(archive_dir)/include \
	-I$(boost_dir) \
	-I$(unicode_dir)/include \
	-I$(unicodeArchive_dir)/include/public \
	-I$(localization_dir)/include \
	-I$(localizationArchive_dir)/include/public \
	-I$(crypto_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-I$(game_dir)/shared/library/swgSharedUtility/include/public \
	-Wno-ctor-dtor-privacy

libserverNetworkMessagessharedcore_la_SOURCES=FirstServerNetworkMessages.h \
	SetupServerNetworkMessages.cpp \
	SetupServerNetworkMessages.h \
	TaskConsoleCommand.cpp \
	TaskConsoleCommand.h


libserverNetworkMessagessharedcore_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
