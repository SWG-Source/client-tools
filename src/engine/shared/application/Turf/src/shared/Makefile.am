AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libTurfshared.la
libTurfshared_la_SOURCES=\
HeightSampler.cpp \
HeightSampler.h \
SamplerProceduralTerrainAppearance.cpp \
SamplerProceduralTerrainAppearance.h \
SamplerProceduralTerrainAppearance_Cache.cpp \
SamplerProceduralTerrainAppearance_Cache.h \
SamplerProceduralTerrainAppearanceTemplate.cpp \
SamplerProceduralTerrainAppearanceTemplate.h \
Turf.cpp \
Turf.h
libTurfshared_la_CXXFLAGS=-I$(srcdir)/../$(platform) \
	-I$(archive_dir)/include \
	-I$(unicode_dir)/include \
	-I$(localization_dir)/include \
   -I$(fileInterface_dir)/include/public \
	-I$(engine_dir)/shared/library/sharedCollision/include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFractal/include/public \
	-I$(engine_dir)/shared/library/sharedGame/include/public \
	-I$(engine_dir)/shared/library/sharedImage/include/public \
	-I$(engine_dir)/shared/library/sharedLog/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryBlockManager/include/public \
	-I$(engine_dir)/shared/library/sharedObject/include/public \
	-I$(engine_dir)/shared/library/sharedPathfinding/include/public \
	-I$(engine_dir)/shared/library/sharedRandom/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedTerrain/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(engine_dir)/shared/library/sharedUtility/include/public \
	-I$(engine_dir)/shared/library/sharedXml/include/public \
	-I$(stlport_dir)/stlport \
	-I$(boost_dir) \
	-Wno-ctor-dtor-privacy

include $(engine_dir)/../make_include/engine.include
