AUTOMAKE_OPTIONS=foreign

noinst_LTLIBRARIES=libsharedTerrainsharedgenerator.la
libsharedTerrainsharedgenerator_la_CXXFLAGS=-I$(srcdir)/../../../include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedFractal/include/public \
	-I$(engine_dir)/shared/library/sharedGame/include/public \
	-I$(engine_dir)/shared/library/sharedImage/include/public \
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
libsharedTerrainsharedgenerator_la_SOURCES=AffectorColor.cpp \
	AffectorColor.h \
	Affector.cpp \
	AffectorEnvironment.cpp \
	AffectorEnvironment.h \
	AffectorExclude.cpp \
	AffectorExclude.h \
	AffectorFloraDynamic.cpp \
	AffectorFloraDynamic.h \
	AffectorFloraStatic.cpp \
	AffectorFloraStatic.h \
	Affector.h \
	AffectorHeight.cpp \
	AffectorHeight.h \
	AffectorPassable.cpp \
	AffectorPassable.h \
	AffectorRibbon.cpp \
	AffectorRibbon.h \
	AffectorRiver.cpp \
	AffectorRiver.h \
	AffectorRoad.cpp \
	AffectorRoad.h \
	AffectorShader.cpp \
	AffectorShader.h \
	Array2d.h \
	BitmapGroup.cpp \
	BitmapGroup.h \
	Boundary.cpp \
	Boundary.h \
	ColorRamp256.cpp \
	ColorRamp256.h \
	CoordinateHash.cpp \
	CoordinateHash.h \
	EnvironmentGroup.cpp \
	EnvironmentGroup.h \
	FastKeyList.h \
	FastList.h \
	Feather.h \
	Filter.cpp \
	Filter.h \
	FloraGroup.cpp \
	FloraGroup.h \
	FractalGroup.cpp \
	FractalGroup.h \
	HeightData.cpp \
	HeightData.h \
	RadialGroup.cpp \
	RadialGroup.h \
	ShaderGroup.cpp \
	ShaderGroup.h \
	TerrainGenerator.cpp \
	TerrainGenerator.h \
	TerrainGeneratorLoader.cpp \
	TerrainGeneratorLoader.h \
	TerrainGeneratorType.def \
	TerrainGeneratorType.h \
	TerrainModificationHelper.cpp \
	TerrainModificationHelper.h


include $(engine_dir)/../make_include/engine.include
