AUTOMAKE_OPTIONS=foreign

noinst_LTLIBRARIES=libsharedTerrainsharedappearance.la
libsharedTerrainsharedappearance_la_CXXFLAGS=-I$(srcdir)/../../../include/public \
	-I$(engine_dir)/shared/library/sharedCollision/include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedGame/include/public \
	-I$(engine_dir)/shared/library/sharedLog/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedMessageDispatch/include/public \
	-I$(engine_dir)/shared/library/sharedObject/include/public \
	-I$(engine_dir)/shared/library/sharedRandom/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedTerrain/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(engine_dir)/shared/library/sharedUtility/include/public \
	 \
	-I$(archive_dir)/include \
	-I$(localization_dir)/include \
	-I$(unicode_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy

libsharedTerrainsharedappearance_la_SOURCES=ProceduralTerrainAppearance.cpp \
	ProceduralTerrainAppearance.h \
	ProceduralTerrainAppearanceTemplate.cpp \
	ProceduralTerrainAppearanceTemplate.h \
	SamplerProceduralTerrainAppearance.cpp \
	SamplerProceduralTerrainAppearance.h \
	SamplerProceduralTerrainAppearance_Cache.cpp \
	SamplerProceduralTerrainAppearance_Cache.h \
	SamplerProceduralTerrainAppearanceTemplate.cpp \
	SamplerProceduralTerrainAppearanceTemplate.h \
	ServerProceduralTerrainAppearance_Cache.cpp \
	ServerProceduralTerrainAppearance_Cache.h \
	ServerProceduralTerrainAppearance.cpp \
	ServerProceduralTerrainAppearance.h \
	ServerProceduralTerrainAppearanceTemplate.cpp \
	ServerProceduralTerrainAppearanceTemplate.h \
	ServerSpaceTerrainAppearance.cpp \
	ServerSpaceTerrainAppearance.h \
	ServerSpaceTerrainAppearanceTemplate.cpp \
	ServerSpaceTerrainAppearanceTemplate.h \
	SpaceTerrainAppearance.cpp \
	SpaceTerrainAppearance.h \
	SpaceTerrainAppearanceTemplate.cpp \
	SpaceTerrainAppearanceTemplate.h \
	TerrainAppearance.cpp \
	TerrainAppearance.h \
	TerrainQuadTree.cpp \
	TerrainQuadTree.h

include $(engine_dir)/../make_include/engine.include
