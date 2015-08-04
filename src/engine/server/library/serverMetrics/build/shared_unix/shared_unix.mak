LIBRARY = serverMetrics

include ../../../../../build/shared_unix/defs.mak
include ../../../../../build/$(PLATFORM)/platformDefs.mak

SHARED_SRC_DIR = ../../../../../shared/library
SERVER_SRC_DIR = ../../..

INCLUDE_DIRS = \
	../../include/public \
	../../include/private \
	../../src/shared \
	../../src/$(PLATFORM) \
	$(DEBUG_INCLUDE_PATH) \
	$(FOUNDATION_INCLUDE_PATH) \
	$(FOUNDATION_TYPES_INCLUDE_PATH) \
	$(MATH_INCLUDE_PATH) \
	$(MEMORY_MANAGER_INCLUDE_PATH) \
	$(MESSAGE_DISPATCH_INCLUDE_PATH) \
	$(NETWORK_MESSAGES_INCLUDE_PATH) \
	$(OBJECT_INCLUDE_PATH) \
	$(SERVER_KEYSHARE_INCLUDE_PATH) \
	$(SERVER_NETWORK_MESSAGES_INCLUDE_PATH) \
	$(SERVER_UTILITY_INCLUDE_PATH) \
	$(STL_INCLUDE_PATH) \
	$(ARCHIVE_PATH)/include \
	$(UNICODE_INCLUDE_PATH) \
	$(MATH_ARCHIVE_INCLUDE_PATH) \
	$(COMMON_ENGINE_LIBRARY_INCLUDES) \
	../../../../../../external/ours/library/unicodeArchive/include \
	../../../../../shared/library/sharedNetwork/include/public \
	../../../../../../external/ours/library/singleton/include \
	../../../../../../external/ours/library/localization/include

include ../../../../../build/shared_unix/library.mak
