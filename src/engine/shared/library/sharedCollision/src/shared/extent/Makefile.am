AUTOMAKE_OPTIONS=foreign

lib_LTLIBRARIES=libsharedCollisionsharedextent.la
libsharedCollisionsharedextent_la_CXXFLAGS=-I$(srcdir)/../../../include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedFractal/include/public \
	-I$(engine_dir)/shared/library/sharedGame/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedMessageDispatch/include/public \
	-I$(engine_dir)/shared/library/sharedObject/include/public \
	-I$(engine_dir)/shared/library/sharedRandom/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedTerrain/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(engine_dir)/shared/library/sharedUtility/include/public \
	 \
	-I$(archive_dir)/include \
	-I$(localization_dir)/include \
	-I$(unicode_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy
libsharedCollisionsharedextent_la_SOURCES=BaseExtent.cpp \
	BaseExtent.h \
	BoxExtent.cpp \
	BoxExtent.h \
	CollisionDetect.cpp \
	CollisionDetect.h \
	ComponentExtent.cpp \
	ComponentExtent.h \
	CompositeExtent.cpp \
	CompositeExtent.h \
	CylinderExtent.cpp \
	CylinderExtent.h \
	DetailExtent.cpp \
	DetailExtent.h \
	Extent.cpp \
	Extent.h \
	ExtentList.cpp \
	ExtentList.h \
	MeshExtent.cpp \
	MeshExtent.h \
	OrientedCylinderExtent.cpp \
	OrientedCylinderExtent.h \
	SimpleExtent.cpp \
	SimpleExtent.h

libsharedCollisionsharedextent_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
