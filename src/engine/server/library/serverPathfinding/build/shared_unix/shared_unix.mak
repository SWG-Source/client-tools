LIBRARY = serverPathfinding

include ../../../../../build/shared_unix/defs.mak
include ../../../../../build/$(PLATFORM)/platformDefs.mak

INCLUDE_DIRS = \
	../../include/public \
	../../include/private \
	../../src/shared \
	../../src/$(PLATFORM) \
	$(ARCHIVE_INCLUDE_PATH) \
	$(COLLISION_INCLUDE_PATH) \
	$(FILE_INCLUDE_PATH) \
	$(GAME_INCLUDE_PATH) \
	$(MATH_ARCHIVE_INCLUDE_PATH) \
	$(MATH_INCLUDE_PATH) \
	$(TERRAIN_INCLUDE_PATH) \
	$(LOCALIZATION_INCLUDE_PATH) \
	$(SYNCHRONIZATION_INCLUDE_PATH) \
	$(MESSAGE_DISPATCH_INCLUDE_PATH) \
	$(NETWORK_MESSAGES_INCLUDE_PATH) \
	$(PATHFINDING_INCLUDE_PATH) \
	$(RANDOM_INCLUDE_PATH) \
	$(SERVER_GAME_INCLUDE_PATH) \
	$(SERVER_SCRIPT_INCLUDE_PATH) \
	$(SERVER_UTILITY_INCLUDE_PATH) \
	$(SKILL_SYSTEM_INCLUDE_PATH) \
	$(SWG_SHARED_UTILITY_INCLUDE_PATH) \
	$(UTILITY_INCLUDE_PATH) \
	$(VNL_INCLUDE_PATH) \
	../../../../../../external/ours/library/singleton/include \
	$(COMMON_ENGINE_LIBRARY_INCLUDES) \
	../../../../../../external/3rd/library/boost

include ../../../../../build/shared_unix/library.mak
