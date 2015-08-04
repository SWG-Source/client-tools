AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedSynchronization.la
libsharedSynchronization_la_LIBADD=src/shared/libsharedSynchronizationshared.la src/${platform}/libsharedSynchronization${platform}.la
libsharedSynchronization_la_SOURCES=src/shared/FirstSharedThread.h
libsharedSynchronization_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
