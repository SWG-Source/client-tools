AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedMemoryManager.la
libsharedMemoryManager_la_LIBADD=src/shared/libsharedMemoryManagershared.la src/${platform}/libsharedMemoryManager${platform}.la
libsharedMemoryManager_la_SOURCES=src/shared/FirstSharedThread.h
libsharedMemoryManager_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
