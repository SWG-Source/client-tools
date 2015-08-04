AUTOMAKE_OPTIONS=foreign
SUBDIRS=src .
lib_LTLIBRARIES=libsharedNetwork.la
libsharedNetwork_la_LIBADD=src/shared/libsharedNetworkshared.la \
	src/shared/UdpLibraryMT/libsharedNetworkUdpLibraryMT.la \
	src/${platform}/libsharedNetwork${platform}.la
libsharedNetwork_la_SOURCES=src/shared/FirstSharedNetwork.h
libsharedNetwork_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
