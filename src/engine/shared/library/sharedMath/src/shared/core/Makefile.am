AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedMathsharedcore.la
libsharedMathsharedcore_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedRandom/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	 \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy
libsharedMathsharedcore_la_SOURCES=AxialBox.cpp \
	AxialBox.h \
	Capsule.cpp \
	Capsule.h \
	Circle.cpp \
	Circle.h \
	Cylinder.cpp \
	Cylinder.h \
	Line3d.cpp \
	Line3d.h \
	MultiShape.cpp \
	MultiShape.h \
	OrientedBox.cpp \
	OrientedBox.h \
	OrientedCircle.cpp \
	OrientedCircle.h \
	OrientedCylinder.cpp \
	OrientedCylinder.h \
	Plane3d.cpp \
	Plane3d.h \
	Quadratic.cpp \
	Quadratic.h \
	Range.cpp \
	Range.h \
	RangeLoop.cpp \
	RangeLoop.h \
	Ray3d.cpp \
	Ray3d.h \
	Ribbon3d.cpp \
	Ribbon3d.h \
	Ring.cpp \
	Ring.h \
	Segment3d.cpp \
	Segment3d.h \
	ShapeUtils.cpp \
	ShapeUtils.h \
	Torus.cpp \
	Torus.h \
	Triangle2d.cpp \
	Triangle2d.h \
	Triangle3d.cpp \
	Triangle3d.h \
	YawedBox.cpp \
	YawedBox.h



include $(engine_dir)/../make_include/engine.include
