AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedMessageDispatch.la
libsharedMessageDispatch_la_LIBADD=src/shared/libsharedMessageDispatchshared.la 
libsharedMessageDispatch_la_SOURCES=src/shared/FirstSharedMessageDispatch.h
libsharedMessageDispatch_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
