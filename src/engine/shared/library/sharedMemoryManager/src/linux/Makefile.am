AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedMemoryManagerlinux.la
libsharedMemoryManagerlinux_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(srcdir)/../../include/private \
	-I$(srcdir)/../../../sharedCompression/include/public \
	-I$(srcdir)/../../../sharedDebug/include/public \
	-I$(srcdir)/../../../sharedFoundation/include/public \
	-I$(srcdir)/../../../sharedFoundationTypes/include/public \
	-I$(srcdir)/../../../sharedMath/include/public \
	-I$(srcdir)/../../../sharedMemoryManager/include/public \
	-I$(srcdir)/../../../sharedSynchronization/include/public \
	-I$(srcdir)/../../../shared/library/sharedThread/include/public \
	 \
	-Wno-ctor-dtor-privacy
libsharedMemoryManagerlinux_la_SOURCES=OsMemory.cpp \
	OsMemory.h \
	OsNewDel.cpp \
	OsNewDel.h 






include $(engine_dir)/../make_include/engine.include
