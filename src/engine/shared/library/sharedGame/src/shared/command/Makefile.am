AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libsharedGamesharedcommand.la
libsharedGamesharedcommand_la_CXXFLAGS=-I$(srcdir)/../../../include/public \
	-I$(srcdir)/../../../include/private \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedLog/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMessageDispatch/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedObject/include/public \
	-I$(engine_dir)/shared/library/sharedRandom/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(engine_dir)/shared/library/sharedUtility/include/public \
	 \
	-I$(udplibrary_dir) \
	-I$(archive_dir)/include \
	-I$(localization_dir)/include \
	-I$(unicode_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-I$(game_dir)/shared/library/swgSharedUtility/include/public \
	-Wno-ctor-dtor-privacy
libsharedGamesharedcommand_la_SOURCES=Command.cpp \
	Command.h \
	CommandChecks.cpp \
	CommandChecks.h \
	CommandCppFunc.def \
	CommandTable.cpp \
	CommandTable.h

libsharedGamesharedcommand_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
