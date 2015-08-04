AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedPathfinding.la
libsharedPathfinding_la_LIBADD=src/shared/libsharedPathfindingshared.la 
libsharedPathfinding_la_SOURCES=src/shared/FirstSharedPathfinding.h
libsharedPathfinding_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
