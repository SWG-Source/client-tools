AUTOMAKE_OPTIONS=foreign
SUBDIRS=src src_oci .
lib_LTLIBRARIES=libsharedDatabaseInterface.la
libsharedDatabaseInterface_la_LIBADD=src/shared/core/libsharedDatabaseInterfacesharedcore.la \
	src/shared/logger/libsharedDatabaseInterfacesharedlogger.la \
	src/shared/queue/libsharedDatabaseInterfacesharedqueue.la \
	src_oci/libsharedDatabaseInterfacesrc_oci.la
libsharedDatabaseInterface_la_SOURCES=src/shared/FirstSharedFoundation.h
libsharedDatabaseInterface_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
