AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedRemoteDebugServer.la
libsharedRemoteDebugServer_la_LIBADD=src/shared/libsharedRemoteDebugServershared.la 
libsharedRemoteDebugServer_la_SOURCES=src/shared/FirstSharedObject.h
libsharedRemoteDebugServer_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
