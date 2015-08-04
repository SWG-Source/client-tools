AUTOMAKE_OPTIONS=foreign

lib_LTLIBRARIES=libsharedObjectsharedproperty.la
libsharedObjectsharedproperty_la_CXXFLAGS=-I$(srcdir)/../../../include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedRandom/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedTerrain/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	 \
	-I$(archive_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy
libsharedObjectsharedproperty_la_SOURCES=CustomizationDataProperty.cpp \
	CustomizationDataProperty.h \
	LayerProperty.cpp \
	LayerProperty.h \
	Property.cpp \
	Property.h \
	PropertyId.h

libsharedObjectsharedproperty_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
