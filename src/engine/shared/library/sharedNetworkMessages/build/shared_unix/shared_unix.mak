LIBRARY = sharedNetworkMessages

include ../../../../../build/shared_unix/defs.mak
include ../../../../../build/$(PLATFORM)/platformDefs.mak

INCLUDE_DIRS = \
	../../include/public \
	../../include/private \
	../../src/shared/clientGameServer \
	../../src/shared/clientLoginServer \
	../../src/shared/common \
	../../src/shared/core \
	../../src/$(PLATFORM) \
	$(DEBUG_INCLUDE_PATH) \
	$(FILE_INCLUDE_PATH) \
	$(FOUNDATION_INCLUDE_PATH) \
	$(FOUNDATION_TYPES_INCLUDE_PATH) \
	$(GAME_INCLUDE_PATH) \
	$(MATH_INCLUDE_PATH) \
	$(MATH_ARCHIVE_INCLUDE_PATH) \
	$(MEMORY_MANAGER_INCLUDE_PATH) \
	$(MESSAGE_DISPATCH_INCLUDE_PATH) \
	$(OBJECT_INCLUDE_PATH) \
	$(RANDOM_INCLUDE_PATH) \
	$(SERVER_GAME_INCLUDE_PATH) \
	$(SYNCHRONIZATION_INCLUDE_PATH) \
	$(UTILITY_INCLUDE_PATH) \
	$(ARCHIVE_PATH)/include \
	$(LOCALIZATION_INCLUDE_PATH) \
	$(BOOST_INCLUDE_PATH) \
	../../../../../../external/ours/library/unicodeArchive/include \
	../../../../../../external/ours/library/unicode/include \
	../../../../../../external/ours/library/singleton/include \
	../../../../../../engine/shared/library/sharedNetwork/include/public \
	$(SWG_SHARED_UTILITY_INCLUDE_PATH) \
	$(COMMON_ENGINE_LIBRARY_INCLUDES)

include ../../../../../build/shared_unix/library.mak
