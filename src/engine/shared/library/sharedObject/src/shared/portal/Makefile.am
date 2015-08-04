AUTOMAKE_OPTIONS=foreign

lib_LTLIBRARIES=libsharedObjectsharedportal.la
libsharedObjectsharedportal_la_CXXFLAGS=-I$(srcdir)/../../../include/public \
	-I$(engine_dir)/shared/library/sharedCollision/include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedPathfinding/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(engine_dir)/shared/library/sharedUtility/include/public \
	 \
	-I$(archive_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy
libsharedObjectsharedportal_la_SOURCES=CellProperty.cpp \
	CellProperty.h \
	Portal.cpp \
	Portal.h \
	PortallizedSphereTree.h \
	PortalProperty.cpp \
	PortalProperty.h \
	PortalPropertyTemplate.cpp \
	PortalPropertyTemplate.h \
	PortalPropertyTemplateList.cpp \
	PortalPropertyTemplateList.h \
	SphereGrid.h

libsharedObjectsharedportal_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
