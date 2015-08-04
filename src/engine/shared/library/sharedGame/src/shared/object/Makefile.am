AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libsharedGamesharedobject.la
libsharedGamesharedobject_la_CXXFLAGS=-I$(srcdir)/../../../include/public \
	-I$(srcdir)/../../../include/private \
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
	-I$(engine_dir)/shared/library/sharedRandom/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedTerrain/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	-I$(engine_dir)/shared/library/sharedUtility/include/public \
	 \
	-I$(boost_dir) \
	-I$(udplibrary_dir) \
	-I$(archive_dir)/include \
	-I$(unicode_dir)/include \
	-I$(unicodeArchive_dir)/include/public \
	-I$(localization_dir)/include \
	-I$(localizationArchive_dir)/include/public \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy
libsharedGamesharedobject_la_SOURCES=AppearanceManager.cpp \
	AppearanceManager.h \
	AttribMod.cpp \
	AttribMod.h \
	AttribModArchive.cpp \
	AttribModArchive.h \
	Buff.cpp \
	Buff.h \
	GroupPickupPoint.cpp \
	GroupPickupPoint.h \
	LotManagerNotification.cpp \
	LotManagerNotification.h \
	MatchMakingCharacterPreferenceId.cpp \
	MatchMakingCharacterPreferenceId.h \
	MatchMakingCharacterProfileId.cpp \
	MatchMakingCharacterProfileId.h \
	MatchMakingCharacterResult.cpp \
	MatchMakingCharacterResult.h \
	MatchMakingId.cpp \
	MatchMakingId.h \
	NoBuildNotification.cpp \
	NoBuildNotification.h \
	ObjectUsabilityManager.cpp \
	ObjectUsabilityManager.h \
	ShipObjectInterface.cpp \
	ShipObjectInterface.h \
	SlopeEffectProperty.cpp \
	SlopeEffectProperty.h \
	SlowDownProperty.cpp \
	SlowDownProperty.h \
	ResourceClassObject.cpp \
	ResourceClassObject.h \
	VehicleHoverDynamics.cpp \
	VehicleHoverDynamics.h \
	Waypoint.cpp \
	WaypointData.cpp \
	WaypointData.h \
	Waypoint.h

libsharedGamesharedobject_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
