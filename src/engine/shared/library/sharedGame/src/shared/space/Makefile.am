AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libsharedGamesharedspace.la
libsharedGamesharedspace_la_CXXFLAGS=-I$(srcdir)/../../../include/public \
	-I$(srcdir)/../../../include/private \
	-I$(engine_dir)/shared/library/sharedCollision/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFile/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedObject/include/public \
	-I$(engine_dir)/shared/library/sharedRandom/include/public \
	-I$(engine_dir)/shared/library/sharedUtility/include/public \
	-I$(engine_dir)/shared/library/sharedMessageDispatch/include/public \
	-I$(archive_dir)/include \
	 \
	-I$(unicode_dir)/include \
	-I$(localization_dir)/include \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy
libsharedGamesharedspace_la_SOURCES=\
	AsteroidGenerationManager.cpp \
	AsteroidGenerationManager.h \
	Nebula.cpp \
	Nebula.h \
	NebulaManager.cpp \
	NebulaManager.h \
	PlayerFormationManager.cpp \
	PlayerFormationManager.h \
	ShipChassis.cpp \
	ShipChassis.h \
	ShipChassisWritable.cpp \
	ShipChassisWritable.h \
	ShipChassisSlot.cpp \
	ShipChassisSlot.h \
	ShipChassisSlotType.cpp \
	ShipChassisSlotType.h \
	ShipComponentAttachmentManager.cpp \
	ShipComponentAttachmentManager.h \
	ShipComponentData.cpp \
	ShipComponentData.h \
	ShipComponentDescriptor.cpp \
	ShipComponentDescriptor.h \
	ShipComponentDescriptorWritable.cpp \
	ShipComponentDescriptorWritable.h \
	ShipComponentFlags.cpp \
	ShipComponentFlags.h \
	ShipComponentType.cpp \
	ShipComponentType.h \
	ShipComponentWeaponManager.cpp \
	ShipComponentWeaponManager.h \
	ShipSlotIdManager.cpp \
	ShipSlotIdManager.h \
	ShipTargeting.cpp \
	ShipTargeting.h \
	ShipTurretManager.cpp \
	ShipTurretManager.h

libsharedGamesharedspace_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
