AUTOMAKE_OPTIONS=foreign

lib_LTLIBRARIES=libsharedObjectsharedobject.la
libsharedObjectsharedobject_la_CXXFLAGS=-I$(srcdir)/../../../include/public \
	-I$(engine_dir)/shared/library/sharedCollision/include/public \
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
libsharedObjectsharedobject_la_SOURCES=AlterResult.cpp \
	AlterResult.h \
	AlterScheduler.cpp \
	AlterScheduler.h \
	CachedNetworkIdArchive.h \
	CachedNetworkId.cpp \
	CachedNetworkId.h \
	DebugNotification.cpp \
	DebugNotification.h \
	DebugYawedFloorNotification.cpp \
	DebugYawedFloorNotification.h \
	MemoryBlockManagedObject.cpp \
	MemoryBlockManagedObject.h \
	NetworkIdManager.cpp \
	NetworkIdManager.h \
	Object.cpp \
	Object.h \
	ObjectList.cpp \
	ObjectList.h \
	ObjectNotification.cpp \
	ObjectNotification.h \
	ObjectTemplate.cpp \
	ObjectTemplate.h \
	ObjectTemplateList.cpp \
	ObjectTemplateList.h \
	ScheduleData.cpp \
	ScheduleData.h

libsharedObjectsharedobject_la_LDFLAGS=-avoid-version
libsharedObjectsharedobject_la_LIBADD=$(top_builddir)/../engine/shared/library/sharedCollision/src/shared/core/libsharedCollisionsharedcore.la
include $(engine_dir)/../make_include/engine.include
