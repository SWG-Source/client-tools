LIBRARY = sharedCommandParser

include ../../../../../build/shared_unix/defs.mak
include ../../../../../build/$(PLATFORM)/platformDefs.mak


INCLUDE_DIRS = \
	../../include/public \
	../../include/private \
	../../src/shared \
	$(OBJECT_PATH)/include/public \
	$(COMMON_ENGINE_LIBRARY_INCLUDES) \
	$(STL_INCLUDE_PATH) \
	$(UNICODE_INCLUDE_PATH)

include ../../../../../build/shared_unix/library.mak
