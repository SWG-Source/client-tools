AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedRegex.la
libsharedRegex_la_LIBADD=src/shared/libsharedRegexshared.la src/${platform}/libsharedRegex${platform}.la
libsharedRegex_la_SOURCES=src/shared/FirstSharedThread.h
libsharedRegex_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
