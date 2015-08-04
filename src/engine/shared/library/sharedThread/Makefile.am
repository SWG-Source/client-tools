AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedThread.la
libsharedThread_la_LIBADD=src/shared/libsharedThreadshared.la src/${platform}/libsharedThread${platform}.la
libsharedThread_la_SOURCES=src/shared/FirstSharedThread.h
libsharedThread_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
