AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedUtility.la
libsharedUtility_la_LIBADD=src/shared/libsharedUtilityshared.la
libsharedUtility_la_SOURCES=src/shared/FirstSharedThread.h
libsharedUtility_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
