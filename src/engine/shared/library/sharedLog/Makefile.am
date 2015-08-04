AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedLog.la
libsharedLog_la_LIBADD=src/shared/libsharedLogshared.la src/${platform}/libsharedLog${platform}.la
libsharedLog_la_SOURCES=src/shared/FirstSharedLog.h
libsharedLog_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
