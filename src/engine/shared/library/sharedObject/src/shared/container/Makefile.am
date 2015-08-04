AUTOMAKE_OPTIONS=foreign

lib_LTLIBRARIES=libsharedObjectsharedcontainer.la
libsharedObjectsharedcontainer_la_CXXFLAGS=-I$(srcdir)/../../../include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	 \
	-I$(archive_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy
libsharedObjectsharedcontainer_la_SOURCES=ArrangementDescriptor.cpp \
	ArrangementDescriptor.h \
	ArrangementDescriptorList.cpp \
	ArrangementDescriptorList.h \
	ContainedByProperty.cpp \
	ContainedByProperty.h \
	Container.cpp \
	Container.h \
	SlotDescriptor.cpp \
	SlotDescriptor.h \
	SlotDescriptorList.cpp \
	SlotDescriptorList.h \
	SlotIdArchive.h \
	SlotId.cpp \
	SlotId.h \
	SlotIdManager.cpp \
	SlotIdManager.h \
	SlottedContainer.cpp \
	SlottedContainer.h \
	SlottedContainmentProperty.cpp \
	SlottedContainmentProperty.h \
	SlotType.def \
	VolumeContainer.cpp \
	VolumeContainer.h \
	VolumeContainmentProperty.cpp \
	VolumeContainmentProperty.h


libsharedObjectsharedcontainer_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
