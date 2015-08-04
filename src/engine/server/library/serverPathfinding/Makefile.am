AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libserverPathfinding.la
libserverPathfinding_la_LIBADD=src/shared/libserverPathfindingshared.la
libserverPathfinding_la_SOURCES=src/shared/FirstServerPathfinding.h
libserverPathfinding_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
