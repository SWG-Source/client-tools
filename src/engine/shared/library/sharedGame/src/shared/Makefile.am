AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedGameshared.la
SUBDIRS=appearance \
	command \
	combat \
	collision \
	core \
	dynamics \
	mount \
	object \
	objectTemplate \
	quest \
	space \
	sui \
	travel
libsharedGameshared_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(srcdir)/../../include/private \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedLog/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMessageDispatch/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	 \
	-I$(udplibrary_dir) \
	-I$(archive_dir)/include \
	-I$(unicode_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy
libsharedGameshared_la_SOURCES=

include $(engine_dir)/../make_include/engine.include
