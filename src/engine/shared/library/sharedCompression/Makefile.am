AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedCompression.la
libsharedCompression_la_LIBADD=src/shared/libsharedCompressionshared.la 
libsharedCompression_la_SOURCES=src/shared/FirstSharedThread.h
libsharedCompression_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
