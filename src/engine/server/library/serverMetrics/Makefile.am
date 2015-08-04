AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libserverMetrics.la
libserverMetrics_la_LIBADD=src/shared/libserverMetricsshared.la
libserverMetrics_la_SOURCES=src/shared/FirstServerMetrics.h
libserverMetrics_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
