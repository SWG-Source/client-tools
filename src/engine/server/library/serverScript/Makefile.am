AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libserverScript.la
libserverScript_la_LIBADD=src/shared/libserverScriptshared.la
libserverScript_la_SOURCES=src/shared/FirstServerPathfinding.h
libserverScript_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
