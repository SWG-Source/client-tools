LIBRARY = sharedTerrain

include ../../../../../build/shared_unix/defs.mak
include ../../../../../build/$(PLATFORM)/platformDefs.mak


INCLUDE_DIRS = \
	../../include/public \
	../../include/private \
	../../src/shared/appearance \
	../../src/shared/core \
	../../src/shared/generator \
	../../src/shared/object \
	../../src/$(PLATFORM) \
	$(COLLISION_INCLUDE_PATH) \
	$(FILE_PATH)/include/public \
	$(FILE_INTERFACE_PATH)/include/public \
	$(FRACTAL_PATH)/include/public \
	$(IMAGE_PATH)/include/public \
	$(MATH_PATH)/include/public \
	$(MESSAGE_DISPATCH_PATH)/include/public \
	$(OBJECT_PATH)/include/public \
	$(NETWORK_MESSAGES_PATH)/include/public \
	$(RANDOM_PATH)/include/public \
	$(SYNCHRONIZATION_PATH)/include/public \
	$(UTILITY_INCLUDE_PATH) \
	../../../../../../external/3rd/library/boost \
	../../../../../../external/ours/library/archive/include \
	../../../../../../external/ours/library/unicode/include \
	$(COMMON_ENGINE_LIBRARY_INCLUDES) \
	$(LOG_INCLUDE_PATH)

include ../../../../../build/shared_unix/library.mak
