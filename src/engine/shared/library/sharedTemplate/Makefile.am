AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedTemplate.la
libsharedTemplate_la_LIBADD=src/shared/core/libsharedTemplatesharedcore.la src/shared/template/libsharedTemplatesharedtemplate.la
libsharedTemplate_la_SOURCES=src/shared/core/FirstSharedTemplate.h
libsharedTemplate_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
