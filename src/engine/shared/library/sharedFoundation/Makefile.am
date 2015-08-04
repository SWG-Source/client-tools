AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedFoundation.la
libsharedFoundation_la_LIBADD=src/shared/libsharedFoundationshared.la \
	src/shared/dynamicVariable/libsharedFoundationshareddynamicvariable.la \
	src/${platform}/libsharedFoundation${platform}.la \
	$(top_builddir)/../engine/shared/library/sharedDebug/libsharedDebug.la

libsharedFoundation_la_SOURCES=src/shared/FirstSharedFoundation.h
libsharedFoundation_la_LDFLAGS=-avoid-version

$(top_builddir)/../engine/shared/library/sharedDebug/libsharedDebug.la:
	cd $(top_builddir)/../engine/shared/library/sharedDebug && $(MAKE)

include $(engine_dir)/../make_include/engine.include
