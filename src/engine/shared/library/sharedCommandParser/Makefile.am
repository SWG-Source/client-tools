AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedCommandParser.la
libsharedCommandParser_la_LIBADD=src/shared/libsharedCommandParsershared.la
libsharedCommandParser_la_SOURCES=src/shared/FirstSharedCommandParser.h
libsharedCommandParser_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
