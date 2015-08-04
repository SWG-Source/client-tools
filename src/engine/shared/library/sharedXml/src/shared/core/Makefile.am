AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedXmlsharedcore.la
libsharedXmlsharedcore_la_CXXFLAGS=-I$(srcdir)/../../../include/public \
	-I$(srcdir)/../../../include/private \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedObject/include/public \
	-I$(engine_dir)/shared/library/sharedRandom/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(boost_dir) \
	 \
	-I$(archive_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-I$(localization_dir)/include \
	-I$(unicode_dir)/include \
	-I$(unicodeArchive_dir)/include/public \
	-I$(libxml_dir)/include \
	-Wno-ctor-dtor-privacy
libsharedXmlsharedcore_la_SOURCES=FirstSharedXml.h \
	SetupSharedXml.cpp \
	SetupSharedXml.h

include $(engine_dir)/../make_include/engine.include
