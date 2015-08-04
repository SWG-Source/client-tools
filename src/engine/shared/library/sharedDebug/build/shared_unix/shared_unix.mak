
LIBRARY = sharedDebug

include ../../../../../build/shared_unix/defs.mak
include ../../../../../build/$(PLATFORM)/platformDefs.mak


INCLUDE_DIRS = \
	../../include/public \
	../../include/private \
	../../src/$(PLATFORM) \
	../../src/shared \
	$(SYNCHRONIZATION_INCLUDE_PATH) \
	$(COMMON_ENGINE_LIBRARY_INCLUDES)

include ../../../../../build/shared_unix/library.mak
