
LIBRARY = sharedLog

include ../../../../../build/shared_unix/defs.mak
include ../../../../../build/$(PLATFORM)/platformDefs.mak


INCLUDE_DIRS = \
	../../include/public \
	../../include/private \
	../../src/$(PLATFORM) \
	../../src/shared \
	$(SYNCHRONIZATION_INCLUDE_PATH) \
	$(FILE_INCLUDE_PATH) \
	$(FILE_INTERFACE_INCLUDE_PATH) \
	$(SHARED_NETWORK_INCLUDE_PATH) \
	$(MESSAGE_DISPATCH_INCLUDE_PATH) \
	$(NETWORK_MESSAGES_INCLUDE_PATH) \
	$(ARCHIVE_INCLUDE_PATH) \
	$(UNICODE_ARCHIVE_INCLUDE_PATH) \
	$(COMMON_ENGINE_LIBRARY_INCLUDES)

include ../../../../../build/shared_unix/library.mak
