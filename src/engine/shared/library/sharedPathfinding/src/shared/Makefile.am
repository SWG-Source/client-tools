AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedPathfindingshared.la
SUBDIRS=
libsharedPathfindingshared_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(srcdir)/../../include/private \
	-I$(engine_dir)/shared/library/sharedCollision/include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedLog/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMessageDispatch/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedObject/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	 \
	-I$(udplibrary_dir) \
	-I$(archive_dir)/include \
	-I$(unicode_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy
libsharedPathfindingshared_la_SOURCES=ConfigSharedPathfinding.cpp \
	ConfigSharedPathfinding.h \
	DynamicPathGraph.cpp \
	DynamicPathGraph.h \
	DynamicPathNode.cpp \
	DynamicPathNode.h \
	FirstSharedPathfinding.h \
	PathEdge.cpp \
	PathEdge.h \
	Pathfinding.cpp \
	PathfindingEnums.h \
	Pathfinding.h \
	PathGraph.cpp \
	PathGraph.h \
	PathGraphIterator.cpp \
	PathGraphIterator.h \
	PathNode.cpp \
	PathNode.h \
	PathSearch.cpp \
	PathSearch.h \
	SetupSharedPathfinding.cpp \
	SetupSharedPathfinding.h \
	SimplePathGraph.cpp \
	SimplePathGraph.h

include $(engine_dir)/../make_include/engine.include
