AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedImage.la
libsharedImage_la_LIBADD=src/shared/libsharedImageshared.la
libsharedImage_la_SOURCES=src/shared/FirstSharedImage.h
libsharedImage_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
