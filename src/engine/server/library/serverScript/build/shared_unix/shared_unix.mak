LIBRARY = serverScript

include ../../../../../build/shared_unix/defs.mak
include ../../../../../build/$(PLATFORM)/platformDefs.mak

SHARED_SRC_DIR = ../../../../../shared/library
SERVER_SRC_DIR = ../../..

INCLUDE_DIRS = \
	../../include/public \
	../../include/private \
	../../src/shared \
	../../src/$(PLATFORM) \
	$(COMMON_ENGINE_LIBRARY_INCLUDES) \
	$(SERVER_GAME_INCLUDE_PATH) \
	$(COLLISION_INCLUDE_PATH) \
	$(DEBUG_INCLUDE_PATH) \
	$(FILE_INCLUDE_PATH) \
	$(FOUNDATION_INCLUDE_PATH) \
	$(FOUNDATION_TYPES_INCLUDE_PATH) \
	$(GAME_INCLUDE_PATH) \
	$(LOG_INCLUDE_PATH) \
	$(MATH_INCLUDE_PATH) \
	$(MEMORY_MANAGER_INCLUDE_PATH) \
	$(SHARED_NETWORK_INCLUDE_PATH) \
	$(SYNCHRONIZATION_INCLUDE_PATH) \
	$(OBJECT_INCLUDE_PATH) \
	$(COMMAND_PARSER_INCLUDE_PATH) \
	$(MATH_ARCHIVE_INCLUDE_PATH) \
	$(MESSAGE_DISPATCH_INCLUDE_PATH) \
	$(PATHFINDING_INCLUDE_PATH) \
	$(SERVER_PATHFINDING_INCLUDE_PATH) \
	$(RANDOM_INCLUDE_PATH) \
	$(UTILITY_INCLUDE_PATH) \
	$(NETWORK_MESSAGES_INCLUDE_PATH) \
	$(LOCALIZATION_INCLUDE_PATH) \
	$(TERRAIN_INCLUDE_PATH) \
	$(THREAD_INCLUDE_PATH) \
	$(SYNCHRONIZATION_INCLUDE_PATH) \
	$(STL_INCLUDE_PATH) \
	$(BOOST_INCLUDE_PATH) \
	/usr/java/include \
	/usr/java/include/linux \
	../../../../../../external/ours/library/fileInterface/include/public \
	../../../../../../external/ours/library/unicode/include \
	../../../../../../external/ours/library/archive/include \
	../../../../../../external/ours/library/unicodeArchive/include \
	../../../../../../game/shared/library/swgSharedUtility/include/public \
	../../../../../../game/shared/library/swgSharedNetworkMessages/include/public \
	../../../../../../game/server/application/SwgGameServer/include/public \
	../../../../../../external/ours/library/singleton/include \
	../../../../../shared/library/sharedSkillSystem/include/public \
	../../../../../server/library/serverUtility/include/public \
	../../../../../server/library/serverNetworkMessages/include/public \
	../../../../../../external/ours/library/vnlv3 \
	../../../../../../external/ours/library/vnlv3/$(PLATFORM)

#@todo codereorg  remove dependancy on serverGame
#also added random and utility, commandparser, messagedispatch, matharchive, unicodearchive, archive because of this

include ../../../../../build/shared_unix/library.mak
