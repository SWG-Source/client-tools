AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libserverKeyShareshared.la
libserverKeyShareshared_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(srcdir)/../../include/private \
	-I$(srcdir)/../../src/$(platform) \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedLog/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMessageDispatch/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedRandom/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(stlport_dir)/stlport \
	-I$(udplibrary_dir) \
	-I$(archive_dir)/include \
	-I$(crypto_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy
libserverKeyShareshared_la_SOURCES=FirstServerKeyShare.h \
	KeyServer.cpp \
	KeyServer.h \
	KeyShare.cpp \
	KeyShare.h

include $(engine_dir)/../make_include/engine.include
