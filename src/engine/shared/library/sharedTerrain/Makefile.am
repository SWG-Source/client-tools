AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedTerrain.la
libsharedTerrain_la_LIBADD=src/shared/appearance/libsharedTerrainsharedappearance.la \
	src/shared/core/libsharedTerrainsharedcore.la \
	src/shared/object/libsharedTerrainsharedobject.la \
	src/shared/generator/libsharedTerrainsharedgenerator.la \
	src/shared/flora/libsharedTerrainsharedflora.la 
libsharedTerrain_la_SOURCES=src/shared/FirstSharedTerrain.h
libsharedTerrain_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
