AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libswgSharedUtilityshared.la
libswgSharedUtilityshared_la_CXXFLAGS=-I$(srcdir)/../../include/public \
	-I$(srcdir)/../../include/private \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedGame/include/public \
	-I$(engine_dir)/shared/library/sharedLog/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMessageDispatch/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedNetwork/include/public \
	-I$(engine_dir)/shared/library/sharedObject/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(engine_dir)/shared/library/sharedUtility/include/public \
	-I$(stlport_dir)/stlport \
	-I$(udplibrary_dir) \
	-I$(singleton_dir)/include \
	-I$(archive_dir)/include \
	-I$(unicode_dir)/include \
	-I$(localization_dir)/include \
	-I$(unicodeArchive_dir)/include/public \
	-I$(localizationArchive_dir)/include/public \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy

libswgSharedUtilityshared_la_SOURCES=Attributes.cpp \
	Attributes.def \
	Attributes.h \
	Behaviors.def \
	CombatEngineData.cpp \
	CombatEngineData.h \
	FirstSwgSharedUtility.h \
	JediConstants.h \
	JediConstants.cpp \
	Locomotions.cpp \
	Locomotions.def \
	Locomotions.h \
	MentalStates.def \
	Postures.cpp \
	Postures.def \
	Postures.h \
	SpeciesRestrictions.cpp \
	SpeciesRestrictions.h \
	States.cpp \
	States.def \
	States.h
