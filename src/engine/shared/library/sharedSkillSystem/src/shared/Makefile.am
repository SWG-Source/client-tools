AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libsharedSkillSystemshared.la
libsharedSkillSystemshared_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(engine_dir)/shared/library/sharedUtility/include/public \
	 \
	-I$(archive_dir)/include \
	-I$(unicode_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy
libsharedSkillSystemshared_la_SOURCES=ExpertiseManager.cpp \
	ExpertiseManager.h \
	LevelManager.cpp \
	LevelManager.h \
	SkillManager.cpp \
	SkillManager.h \
	SkillObjectArchive.cpp \
	SkillObjectArchive.h \
	SkillObject.cpp \
	SkillObject.h


include $(engine_dir)/../make_include/engine.include
