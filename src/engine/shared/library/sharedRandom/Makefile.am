AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedRandom.la
libsharedRandom_la_LIBADD=src/shared/libsharedRandomshared.la 
libsharedRandom_la_SOURCES=src/shared/FirstSharedRandom.h
libsharedRandom_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
