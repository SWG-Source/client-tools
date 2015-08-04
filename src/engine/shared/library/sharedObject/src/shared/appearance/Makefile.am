AUTOMAKE_OPTIONS=foreign

lib_LTLIBRARIES=libsharedObjectsharedappearance.la
libsharedObjectsharedappearance_la_CXXFLAGS=-I$(srcdir)/../../../include/public \
	-I$(engine_dir)/shared/library/sharedCollision/include/public \
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
libsharedObjectsharedappearance_la_SOURCES=Appearance.cpp \
	Appearance.h \
	AppearanceTemplate.cpp \
	AppearanceTemplate.h \
	AppearanceTemplateList.cpp \
	AppearanceTemplateList.h \
	Hardpoint.cpp \
	Hardpoint.h \
	NoRenderAppearance.cpp \
	NoRenderAppearance.h \
	NoRenderAppearanceTemplate.cpp \
	NoRenderAppearanceTemplate.h \
	TextAppearance.cpp \
	TextAppearance.h \
	Tweakable.cpp \
	Tweakable.h


libsharedObjectsharedappearance_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
