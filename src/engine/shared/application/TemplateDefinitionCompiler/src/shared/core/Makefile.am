AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libTemplateDefinitionCompilersharedcore.la
libTemplateDefinitionCompilersharedcore_la_CXXFLAGS=-I$(srcdir)/../../$(platform) \
	-I$(archive_dir)/include \
	-I$(unicode_dir)/include \
	-I$(localization_dir)/include \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedObject/include/public \
	-I$(engine_dir)/shared/library/sharedRandom/include/public \
	-I$(engine_dir)/shared/library/sharedRegex/include/public \
	-I$(engine_dir)/shared/library/sharedTemplate/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(engine_dir)/shared/library/sharedUtility/include/public \
	-I$(engine_dir)/shared/library/sharedTemplateDefinition/include/public \
	-I$(stlport_dir)/stlport \
	-I$(perforce_dir)/include \
	-I$(pcre_dir)/include \
	-Wno-ctor-dtor-privacy
libTemplateDefinitionCompilersharedcore_la_SOURCES=TemplateDefinitionCompiler.cpp


include $(engine_dir)/../make_include/engine.include
