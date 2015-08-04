AUTOMAKE_OPTIONS=foreign

lib_LTLIBRARIES=libsharedObjectsharedcustomization.la
libsharedObjectsharedcustomization_la_CXXFLAGS=-I$(srcdir)/../../../include/public \
	-I$(srcdir)/../../../include/private \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedLog/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	 \
	-I$(archive_dir)/include \
	-I$(unicode_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy
libsharedObjectsharedcustomization_la_SOURCES=BasicRangedIntCustomizationVariable.cpp \
	BasicRangedIntCustomizationVariable.h \
	CustomizationData.cpp \
	CustomizationData_Directory.h \
	CustomizationData.h \
	CustomizationData_LocalDirectory.cpp \
	CustomizationData_LocalDirectory.h \
	CustomizationData_RemoteDirectory.cpp \
	CustomizationData_RemoteDirectory.h \
	CustomizationIdManager.cpp \
	CustomizationIdManager.h \
	CustomizationVariable.cpp \
	CustomizationVariable.h \
	ObjectTemplateCustomizationDataWriter.cpp \
	ObjectTemplateCustomizationDataWriter.h \
	PaletteColorCustomizationVariable.cpp \
	PaletteColorCustomizationVariable.h \
	RangedIntCustomizationVariable.cpp \
	RangedIntCustomizationVariable.h

libsharedObjectsharedcustomization_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
