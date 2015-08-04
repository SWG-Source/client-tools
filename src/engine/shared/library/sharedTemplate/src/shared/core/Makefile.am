AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedTemplatesharedcore.la
libsharedTemplatesharedcore_la_CXXFLAGS=-I$(srcdir)/../../../include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedRandom/include/public \
	-I$(engine_dir)/shared/library/sharedRegex/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedTemplateDefinition/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(engine_dir)/shared/library/sharedUtility/include/public \
	 \
	-I$(archive_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-I$(localization_dir)/include \
	-I$(unicode_dir)/include \
	-Wno-ctor-dtor-privacy
libsharedTemplatesharedcore_la_SOURCES=FirstSharedTemplate.h \
	SetupSharedTemplate.cpp \
	SetupSharedTemplate.h

include $(engine_dir)/../make_include/engine.include
