AUTOMAKE_OPTIONS=foreign

lib_LTLIBRARIES=libsharedObjectshareddynamics.la
libsharedObjectshareddynamics_la_CXXFLAGS=-I$(srcdir)/../../../include/public \
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
libsharedObjectshareddynamics_la_SOURCES=Dynamics.cpp \
	Dynamics.h \
	RotationDynamics.cpp \
	RotationDynamics.h \
	SeesawDynamics.cpp \
	SeesawDynamics.h \
	SimpleDynamics.cpp \
	SimpleDynamics.h \
	SpringDynamics.cpp \
	SpringDynamics.h \
	StateDynamics.cpp \
	StateDynamics.h \
	TrackingDynamics.cpp \
	TrackingDynamics.h

libsharedObjectshareddynamics_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
