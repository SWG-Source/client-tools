AUTOMAKE_OPTIONS=foreign

lib_LTLIBRARIES=libsharedNetworkMessagessharedcore.la
libsharedNetworkMessagessharedcore_la_CXXFLAGS=-I$(srcdir)/../../../include/public \
	-I$(srcdir)/../../../include/private \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedGame/include/public \
	-I$(engine_dir)/shared/library/sharedLog/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMathArchive/include/public \
	-I$(engine_dir)/shared/library/sharedMessageDispatch/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(engine_dir)/shared/library/sharedUtility/include/public \
	 \
	-I$(udplibrary_dir) \
	-I$(archive_dir)/include \
	-I$(unicode_dir)/include \
	-I$(localization_dir)/include \
	-I$(unicodeArchive_dir)/include/public \
	-I$(localizationArchive_dir)/include/public \
	-I$(fileInterface_dir)/include/public \
	-I$(boost_dir) \
	-Wno-ctor-dtor-privacy

libsharedNetworkMessagessharedcore_la_SOURCES=ControllerMessageMacros.h \
	FirstSharedNetworkMessages.h \
	NetworkStringIds.h \
	SetupSharedNetworkMessages.cpp \
	SetupSharedNetworkMessages.h

libsharedNetworkMessagessharedcore_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
