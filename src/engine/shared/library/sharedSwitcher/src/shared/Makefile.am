AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedSwitchershared.la
libsharedSwitchershared_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedRandom/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	 \
	-I$(archive_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy
libsharedSwitchershared_la_SOURCES=DeltaFrameSwitcher.cpp \
	DeltaFrameSwitcher.h \
	DeltaFrameSwitcherTemplate.cpp \
	DeltaFrameSwitcherTemplate.h \
	DeltaRandomFrameSwitcher.cpp \
	DeltaRandomFrameSwitcher.h \
	DeltaRandomFrameSwitcherTemplate.cpp \
	DeltaRandomFrameSwitcherTemplate.h \
	DeltaRandomTimeSwitcher.cpp \
	DeltaRandomTimeSwitcher.h \
	DeltaRandomTimeSwitcherTemplate.cpp \
	DeltaRandomTimeSwitcherTemplate.h \
	DeltaTimeSwitcher.cpp \
	DeltaTimeSwitcher.h \
	DeltaTimeSwitcherTemplate.cpp \
	DeltaTimeSwitcherTemplate.h \
	FirstSharedSwitcher.h \
	Switcher.cpp \
	Switcher.h \
	SwitcherTemplate.cpp \
	SwitcherTemplate.h

include $(engine_dir)/../make_include/engine.include
