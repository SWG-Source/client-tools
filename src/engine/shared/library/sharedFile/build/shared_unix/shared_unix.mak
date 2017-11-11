LIBRARY = sharedFile

include ../../../../../build/shared_unix/defs.mak
include ../../../../../build/$(PLATFORM)/platformDefs.mak


INCLUDE_DIRS = \
	../../include/public \
	../../include/private \
	../../src/shared \
	../../src/$(PLATFORM) \
	$(COMPRESSION_PATH)/include/public \
	$(FILE_INTERFACE_INCLUDE_PATH) \
	$(MATH_PATH)/include/public \
	$(SYNCHRONIZATION_INCLUDE_PATH) \
	$(RANDOM_PATH)/include/public \
	$(SYNCHRONIZATION_PATH)/include/public \
	$(THREAD_PATH)/include/public \
	$(COMMON_ENGINE_LIBRARY_INCLUDES)

include ../../../../../build/shared_unix/library.mak