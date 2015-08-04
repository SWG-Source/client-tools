LIBRARY = sharedNetwork

include ../../../../../build/shared_unix/defs.mak
include ../../../../../build/$(PLATFORM)/platformDefs.mak

INCLUDE_DIRS = \
	../../src/shared \
	../../include/public \
	../../include/private \
	../../src/shared/clientGameServer \
	../../src/shared/clientLoginServer \
	../../src/shared/common \
	../../src/shared/core \
	../../src/$(PLATFORM) \
	$(FOUNDATION_INCLUDE_PATH) \
	$(FOUNDATION_TYPES_INCLUDE_PATH) \
	$(DEBUG_INCLUDE_PATH) \
	$(FILE_INCLUDE_PATH) \
	$(MATH_INCLUDE_PATH) \
	$(MATH_ARCHIVE_INCLUDE_PATH) \
	$(MEMORY_MANAGER_INCLUDE_PATH) \
	$(SYNCHRONIZATION_INCLUDE_PATH) \
	$(MESSAGE_DISPATCH_INCLUDE_PATH) \
	$(OBJECT_INCLUDE_PATH) \
	$(COMPRESSION_INCLUDE_PATH) \
	$(RANDOM_INCLUDE_PATH) \
	$(SERVER_GAME_INCLUDE_PATH) \
	$(SWG_SERVER_NETWORK_MESSAGES_INCLUDE_PATH) \
	$(SWG_SHARED_NETWORK_MESSAGES_INCLUDE_PATH) \
	$(THREAD_INCLUDE_PATH) \
	$(UTILITY_INCLUDE_PATH) \
	$(ARCHIVE_PATH)/include \
	$(LOCALIZATION_INCLUDE_PATH) \
	../../../../../../external/ours/library/unicodeArchive/include \
	../../../../../../external/ours/library/unicode/include \
	../../../../../../game/shared/library/swgSharedUtility/include/public \
	../../../../../../game/server/application/SwgGameServer/include/public \
	../../../../../../external/ours/library/singleton/include \
	../../../../../../external/3rd/library/udplibrary \
	$(COMMON_ENGINE_LIBRARY_INCLUDES) \
	$(LOG_INCLUDE_PATH)

include ../../../../../build/shared_unix/library.mak
