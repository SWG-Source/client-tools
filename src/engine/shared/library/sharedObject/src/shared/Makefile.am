AUTOMAKE_OPTIONS=foreign
SUBDIRS=appearance \
	container \
	controller \
	core \
	customization \
	dynamics \
	lot \
	movement \
	object \
	portal \
	property \
	world

lib_LTLIBRARIES=libsharedObjectshared.la

libsharedObjectshared_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	 \
	-I$(archive_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy
libsharedObjectshared_la_SOURCES=ObjectWatcherList.cpp \
	ObjectWatcherList.h

libsharedObjectshared_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
