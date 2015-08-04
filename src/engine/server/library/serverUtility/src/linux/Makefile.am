AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libserverUtilitylinux.la
libserverUtilitylinux_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(srcdir)/../../include/private \
	-I$(srcdir)/../../src/$(platform) \
	-I$(engine_dir)/server/library/serverUtility/include/public \
	-I$(engine_dir)/server/library/serverNetworkMessages/include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedLog/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMessageDispatch/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedNetwork/include/public \
	-I$(engine_dir)/shared/library/sharedNetworkMessages/include/public \
	-I$(engine_dir)/shared/library/sharedRandom/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(stlport_dir)/stlport \
	-I$(singleton_dir)/include \
	-I$(udplibrary_dir) \
	-I$(archive_dir)/include \
	-I$(crypto_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy
libserverUtilitylinux_la_SOURCES=stlhack.cpp




include $(engine_dir)/../make_include/engine.include
