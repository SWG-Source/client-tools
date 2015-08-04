AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedMath.la
libsharedMath_la_LIBADD=src/shared/libsharedMathshared.la \
	 src/shared/core/libsharedMathsharedcore.la \
	 src/shared/debug/libsharedMathshareddebug.la 
libsharedMath_la_SOURCES=src/shared/FirstSharedThread.h
libsharedMath_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
