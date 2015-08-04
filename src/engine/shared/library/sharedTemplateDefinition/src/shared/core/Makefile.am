AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedTemplateDefinitionsharedcore.la
libsharedTemplateDefinitionsharedcore_la_CXXFLAGS=-I$(srcdir)/../../../include/public \
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
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(engine_dir)/shared/library/sharedUtility/include/public \
	 \
	-I$(archive_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-I$(localization_dir)/include \
	-I$(unicode_dir)/include \
	-I$(pcre_dir)/include \
	-Wno-ctor-dtor-privacy
libsharedTemplateDefinitionsharedcore_la_SOURCES=CompilerDynamicVariableParamData.cpp \
	CompilerDynamicVariableParamData.h \
	CompilerIntegerParam.cpp \
	CompilerIntegerParam.h \
	File.cpp \
	File.h \
	Filename.cpp \
	Filename.h \
	FirstSharedTemplateDefinition.h \
	ObjectTemplate.cpp \
	ObjectTemplate.h \
	TemplateData.cpp \
	TemplateData.h \
	TemplateDataIterator.cpp \
	TemplateDataIterator.h \
	TemplateDefinitionFile.cpp \
	TemplateDefinitionFile.h \
	TemplateGlobals.cpp \
	TemplateGlobals.h \
	TpfFile.cpp \
	TpfFile.h \
	TpfTemplate.cpp \
	TpfTemplate.h

include $(engine_dir)/../make_include/engine.include
