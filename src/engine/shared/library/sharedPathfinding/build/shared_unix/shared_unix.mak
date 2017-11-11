LIBRARY = sharedPathfinding

include ../../../../../build/shared_unix/defs.mak
include ../../../../../build/$(PLATFORM)/platformDefs.mak

INCLUDE_DIRS = \
	../../include/public \
	../../include/private \
	../../src/shared \
	../../src/$(PLATFORM) \
	$(COLLISION_INCLUDE_PATH) \
	$(FILE_INCLUDE_PATH) \
	$(MATH_INCLUDE_PATH) \
	$(OBJECT_INCLUDE_PATH) \
	$(SYNCHRONIZATION_INCLUDE_PATH) \
	$(TERRAIN_INCLUDE_PATH) \
	$(COMMON_ENGINE_LIBRARY_INCLUDES)

include ../../../../../build/shared_unix/library.mak
