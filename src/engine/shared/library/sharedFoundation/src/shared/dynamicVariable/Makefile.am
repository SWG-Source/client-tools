AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedFoundationshareddynamicvariable.la
libsharedFoundationshareddynamicvariable_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMathArchive/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	 \
	-I$(fileInterface_dir)/include/public \
	-I$(archive_dir)/include \
	-I$(localization_dir)/include \
	-I$(unicode_dir)/include \
	-I$(unicodeArchive_dir)/include/public \
	-Wno-ctor-dtor-privacy
libsharedFoundationshareddynamicvariable_la_SOURCES=DynamicVariable.cpp \
	DynamicVariable.h \
	DynamicVariableList.cpp \
	DynamicVariableList.h \
	DynamicVariableListNestedList.cpp \
	DynamicVariableListNestedList.h \
	DynamicVariableLocationData.h


include $(engine_dir)/../make_include/engine.include
