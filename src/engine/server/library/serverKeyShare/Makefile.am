AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libserverKeyShare.la
libserverKeyShare_la_LIBADD=src/shared/libserverKeyShareshared.la
libserverKeyShare_la_SOURCES=src/shared/FirstServerKeyShare.h
libserverKeyShare_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
