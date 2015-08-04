AUTOMAKE_OPTIONS=foreign

lib_LTLIBRARIES=libsharedObjectsharedcore.la
libsharedObjectsharedcore_la_CXXFLAGS=-I$(srcdir)/../../../include/public \
	-I$(engine_dir)/shared/library/sharedCollision/include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(engine_dir)/shared/library/sharedUtility/include/public \
	 \
	-I$(archive_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-I$(game_dir)/shared/library/swgSharedUtility/include/public \
	-Wno-ctor-dtor-privacy
libsharedObjectsharedcore_la_SOURCES=ConfigSharedObject.cpp \
	ConfigSharedObject.h \
	FirstSharedObject.h \
	SetupSharedObject.cpp \
	SetupSharedObject.h

libsharedObjectsharedcore_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
