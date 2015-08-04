AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedTemplateDefinition.la
libsharedTemplateDefinition_la_LIBADD=src/shared/core/libsharedTemplateDefinitionsharedcore.la 
libsharedTemplateDefinition_la_SOURCES=src/shared/core/FirstSharedTemplateDefinition.h
libsharedTemplateDefinition_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
