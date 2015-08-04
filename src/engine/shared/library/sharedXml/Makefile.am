AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedXml.la
libsharedXml_la_LIBADD=src/shared/core/libsharedXmlsharedcore.la src/shared/tree/libsharedXmlsharedtree.la
libsharedXml_la_SOURCES=src/shared/core/FirstSharedXml.h
libsharedXml_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
