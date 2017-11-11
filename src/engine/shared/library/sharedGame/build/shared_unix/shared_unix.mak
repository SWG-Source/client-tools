LIBRARY = sharedGame

include ../../../../../build/shared_unix/defs.mak
include ../../../../../build/$(PLATFORM)/platformDefs.mak

INCLUDE_DIRS = \
	../../include/public \
	../../include/private \
	../../src/shared/core \
	../../src/shared/objectTemplate \
	../../src/$(PLATFORM) \
	$(TERRAIN_INCLUDE_PATH) \
	$(FILE_INCLUDE_PATH) \
	$(FILE_INTERFACE_INCLUDE_PATH) \
	$(LOCALIZATION_INCLUDE_PATH) \
	$(LOG_INCLUDE_PATH) \
	$(MATH_ARCHIVE_INCLUDE_PATH) \
	$(MATH_INCLUDE_PATH) \
	$(MESSAGE_DISPATCH_INCLUDE_PATH) \
	$(NETWORK_MESSAGES_INCLUDE_PATH) \
	$(OBJECT_INCLUDE_PATH) \
	$(RANDOM_INCLUDE_PATH) \
	$(SKILL_SYSTEM_INCLUDE_PATH) \
	$(SYNCHRONIZATION_INCLUDE_PATH) \
	$(UNICODE_INCLUDE_PATH) \
	$(UTILITY_INCLUDE_PATH) \
	$(COLLISION_INCLUDE_PATH) \
	../../../../../../external/3rd/library/boost \
	../../../../../../external/3rd/library/regex \
	../../../../../../external/ours/library/archive/include \
	../../../../../../external/ours/library/singleton/include \
	../../../../../../game/shared/library/swgSharedUtility/include/public \
	$(COMMON_ENGINE_LIBRARY_INCLUDES)

include ../../../../../build/shared_unix/library.mak
