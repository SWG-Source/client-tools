AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedLogshared.la
libsharedLogshared_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedMessageDispatch/include/public \
	-I$(engine_dir)/shared/library/sharedNetwork/include/public \
	-I$(engine_dir)/shared/library/sharedNetworkMessages/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	 \
	-I$(archive_dir)/include \
	-I$(unicode_dir)/include \
	-I$(unicodeArchive_dir)/include/public \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy
libsharedLogshared_la_SOURCES=ConfigSharedLog.cpp \
	ConfigSharedLog.h \
	TailFileLogObserver.cpp \
	TailFileLogObserver.h \
	FileLogObserver.cpp \
	FileLogObserver.h \
	FirstSharedLog.cpp \
	FirstSharedLog.h \
	Log.h \
	LogManager.cpp \
	LogManager.h \
	LogObserver.cpp \
	LogObserver.h \
	NetLogConnection.cpp \
	NetLogConnection.h \
	NetLogObserver.cpp \
	NetLogObserver.h \
	SetupSharedLog.cpp \
	SetupSharedLog.h \
	StderrLogger.h


include $(engine_dir)/../make_include/engine.include
