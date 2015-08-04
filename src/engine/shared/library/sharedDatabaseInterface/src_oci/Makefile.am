AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libsharedDatabaseInterfacesrc_oci.la
libsharedDatabaseInterfacesrc_oci_la_CXXFLAGS=-I$(srcdir)/../include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedLog/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	 \
	-I$(oracle_dir)/include \
	-I$(archive_dir)/include \
	-I$(unicode_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy
libsharedDatabaseInterfacesrc_oci_la_SOURCES=DbBindableVarray.cpp \
	DbBindableVarray.h \
	OciQueryImplementation.cpp \
	OciQueryImplementation.h \
	OciServer.cpp \
	OciServer.h \
	OciSession.cpp \
	OciSession.h

libsharedDatabaseInterfacesrc_oci_la_LDFLAGS=-avoid-version

libsharedDatabaseInterfacesrc_oci_la_LIBADD=$(top_builddir)/../engine/shared/library/sharedDatabaseInterface/src/shared/core/libsharedDatabaseInterfacesharedcore.la

include $(engine_dir)/../make_include/engine.include
