AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedFile.la
libsharedFile_la_LIBADD=src/shared/libsharedFileshared.la src/${platform}/libsharedFile${platform}.la
libsharedFile_la_SOURCES=src/shared/FirstSharedFile.h
libsharedFile_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
