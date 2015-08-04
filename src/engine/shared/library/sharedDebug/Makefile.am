AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedDebug.la
libsharedDebug_la_LIBADD=src/shared/libsharedDebugshared.la \
	src/${platform}/libsharedDebug${platform}.la

libsharedDebug_la_SOURCES=src/shared/FirstSharedDebug.h
libsharedDebug_la_LDFLAGS=-avoid-version


include $(engine_dir)/../make_include/engine.include
