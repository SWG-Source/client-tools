lib_LTLIBRARIES=libmonapi.la
libmonapi_la_SOURCES=MonitorAPI.cpp \
	MonitorAPI.h \
	MonitorData.cpp \
	MonitorData.h
libmonapi_la_CXXFLAGS=-I@udplibrary_dir@ -DMonitorAPI= -Wno-unused
EXTRA_DIST=README
libmonapi_la_LDFLAGS=-avoid-version
