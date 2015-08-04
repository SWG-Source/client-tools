AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libserverUtility.la
libserverUtility_la_LIBADD=src/shared/libserverUtilityshared.la src/$(platform)/libserverUtility$(platform).la
libserverUtility_la_SOURCES=src/shared/FirstServerUtility.h
libserverUtility_la_LDFLAGS=-avoid-version
include $(engine_dir)/../make_include/engine.include
