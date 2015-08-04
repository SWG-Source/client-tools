AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedFileshared.la
libsharedFileshared_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	 \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy
libsharedFileshared_la_SOURCES=AsynchronousLoader.cpp \
	AsynchronousLoader.h \
	ConfigSharedFile.cpp \
	ConfigSharedFile.h \
	FileManifest.cpp \
	FileManifest.h \
	FileNameUtils.cpp \
	FileNameUtils.h \
	FileStreamer.cpp \
	FileStreamerFile.cpp \
	FileStreamerFile.h \
	FileStreamer.h \
	FileStreamerThread.cpp \
	FileStreamerThread.h \
	FirstSharedFile.h \
	Iff.cpp \
	Iff.h \
	MemoryFile.cpp \
	MemoryFile.h \
	SetupSharedFile.cpp \
	SetupSharedFile.h \
	TreeFile.cpp \
	TreeFile.h \
	TreeFile_SearchNode.cpp \
	TreeFile_SearchNode.h \
	ZlibFile.cpp \
	ZlibFile.h

include $(engine_dir)/../make_include/engine.include
