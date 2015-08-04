AUTOMAKE_OPTIONS=foreign
SUBDIRS=chat \
	clientGameServer \
	clientLoginServer \
	common \
	core \
	customerService \
	planetWatch \
	voicechat

lib_LTLIBRARIES=libsharedNetworkMessagesshared.la
libsharedNetworkMessagesshared_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(srcdir)/../../include/private \
	-I$(srcdir)/../../src/$(platform) \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedLog/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMessageDispatch/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedNetwork/include/public \
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
	-Wno-ctor-dtor-privacy

libsharedNetworkMessagesshared_la_SOURCES=GrantCommand.cpp \
	GrantCommand.h \
	GrantSkill.cpp \
	GrantSkill.h \
	LogMessage.cpp \
	LogMessage.h \
	NameErrors.cpp \
	NameErrors.h \
	NetworkMessageFactory.cpp \
	NetworkMessageFactory.h \
	RevokeCommand.cpp \
	RevokeCommand.h \
	RevokeSkill.cpp \
	RevokeSkill.h

libsharedNetworkMessagesshared_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
