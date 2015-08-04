AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedInputMapshared.la
libsharedInputMapshared_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedIoWin/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	 \
	-I$(archive_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy
libsharedInputMapshared_la_SOURCES=FirstSharedInputMap.h \
	InputMap_Command.cpp \
	InputMap_Command.h \
	InputMap.cpp \
	InputMap.h \
	InputMap_Shifts.cpp \
	InputMap_Shifts.h \
	InputMap_SparseMap.cpp \
	InputMap_SparseMap.h

include $(engine_dir)/../make_include/engine.include
