LIBRARY = sharedTemplate

include ../../../../../build/shared_unix/defs.mak
include ../../../../../build/$(PLATFORM)/platformDefs.mak


INCLUDE_DIRS = \
	../../include/public \
	../../include/private \
	../../src/$(PLATFORM) \
	../../src/shared \
	../../../../../../external/3rd/library/perforce/include \
	../../../../library/sharedTemplateDefinition/include/public \
	$(SYNCHRONIZATION_INCLUDE_PATH) \
	$(REGEX_INCLUDE_PATH) \
	$(STL_INCLUDE_PATH) \
	$(FILE_INCLUDE_PATH) \
	$(FILE_INTERFACE_INCLUDE_PATH) \
	$(SHARED_NETWORK_INCLUDE_PATH) \
	$(UTILITY_INCLUDE_PATH) \
	$(RANDOM_INCLUDE_PATH) \
	$(MESSAGE_DISPATCH_INCLUDE_PATH) \
	$(NETWORK_MESSAGES_INCLUDE_PATH) \
	$(ARCHIVE_INCLUDE_PATH) \
	$(MATH_INCLUDE_PATH) \
	$(MATH_ARCHIVE_INCLUDE_PATH) \
	$(UNICODE_ARCHIVE_INCLUDE_PATH) \
	$(COMMON_ENGINE_LIBRARY_INCLUDES)


include ../../../../../build/shared_unix/library.mak
