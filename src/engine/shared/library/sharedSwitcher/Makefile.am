AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedSwitcher.la
libsharedSwitcher_la_LIBADD=src/shared/libsharedSwitchershared.la 
libsharedSwitcher_la_SOURCES=src/shared/FirstSharedObject.h
libsharedSwitcher_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
