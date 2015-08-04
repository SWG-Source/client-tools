AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libserverDatabase.la
libserverDatabase_la_LIBADD=src/shared/libserverDatabaseshared.la
libserverDatabase_la_SOURCES=src/shared/FirstServerUtility.h
libserverDatabase_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
