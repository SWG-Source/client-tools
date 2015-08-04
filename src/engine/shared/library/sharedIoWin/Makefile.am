AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedIoWin.la
libsharedIoWin_la_LIBADD=src/shared/libsharedIoWinshared.la 
libsharedIoWin_la_SOURCES=src/shared/FirstSharedIoWin.h
libsharedIoWin_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
