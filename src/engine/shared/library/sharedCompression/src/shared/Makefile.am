AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedCompressionshared.la
 libsharedCompressionshared_la_CXXFLAGS=-I$(srcdir)/../../include/public \
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
libsharedCompressionshared_la_SOURCES=BitStream.cpp \
	BitStream.h \
	Compressor.cpp \
	Compressor.h \
	FirstSharedCompression.h \
	Lz77.cpp \
	Lz77.h \
	SetupSharedCompression.cpp \
	SetupSharedCompression.h \
	ZlibCompressor.cpp \
	ZlibCompressor.h 


include $(engine_dir)/../make_include/engine.include
