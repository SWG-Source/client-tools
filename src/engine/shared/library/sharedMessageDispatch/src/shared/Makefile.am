AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedMessageDispatchshared.la
libsharedMessageDispatchshared_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(srcdir)/../../include/private \
	-I$(srcdir)/../../src/$(platform) \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedLog/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	 \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy
libsharedMessageDispatchshared_la_SOURCES=Emitter.cpp \
	Emitter.h \
	FirstSharedMessageDispatch.h \
	Message.cpp \
	Message.h \
	MessageManager.cpp \
	MessageManager.h \
	Receiver.cpp \
	Receiver.h \
	Transceiver.cpp \
	Transceiver.h


include $(engine_dir)/../make_include/engine.include
