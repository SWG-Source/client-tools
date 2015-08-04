LIBRARY = serverGame

include ../../../../../build/shared_unix/defs.mak
include ../../../../../build/$(PLATFORM)/platformDefs.mak

SHARED_SRC_DIR = ../../../../../shared/library
SERVER_SRC_DIR = ../../..

INCLUDE_DIRS = \
	../../include/public \
	../../include/private \
	../../src/shared/behavior \
	../../src/shared/core \
	../../src/shared/controller \
	../../src/shared/object \
	../../src/shared/console \
	../../src/shared/network \
	../../src/shared/objectTemplate \
	../../src/$(PLATFORM) \
	$(COMMON_ENGINE_LIBRARY_INCLUDES) \
	$(ARCHIVE_PATH)/include \
	$(COLLISION_INCLUDE_PATH) \
	$(COMMAND_PARSER_INCLUDE_PATH) \
	$(DEBUG_INCLUDE_PATH) \
	$(FILE_INCLUDE_PATH) \
	$(FILE_INTERFACE_INCLUDE_PATH) \
	$(FOUNDATION_INCLUDE_PATH) \
	$(FOUNDATION_TYPES_INCLUDE_PATH) \
	$(FRACTAL_INCLUDE_PATH) \
	$(GAME_INCLUDE_PATH) \
	$(LOCALIZATION_INCLUDE_PATH) \
	$(LOG_INCLUDE_PATH) \
	$(MATH_ARCHIVE_INCLUDE_PATH) \
	$(MATH_INCLUDE_PATH) \
	$(MEMORY_MANAGER_INCLUDE_PATH) \
	$(MESSAGE_DISPATCH_INCLUDE_PATH) \
	$(NETWORK_MESSAGES_INCLUDE_PATH) \
	$(OBJECT_INCLUDE_PATH) \
	$(PATHFINDING_INCLUDE_PATH) \
	$(RANDOM_INCLUDE_PATH) \
	$(SERVER_METRICS_INCLUDE_PATH) \
	$(SERVER_NETWORK_MESSAGES_INCLUDE_PATH) \
	$(SERVER_PATHFINDING_INCLUDE_PATH) \
	$(SERVER_SCRIPT_INCLUDE_PATH) \
	$(SERVER_UTILITY_INCLUDE_PATH) \
	$(SKILL_SYSTEM_INCLUDE_PATH) \
	$(STL_INCLUDE_PATH) \
	$(SWG_SERVER_NETWORK_MESSAGES_INCLUDE_PATH) \
	$(SWG_SHARED_NETWORK_MESSAGES_INCLUDE_PATH) \
	$(SYNCHRONIZATION_INCLUDE_PATH) \
	$(TERRAIN_INCLUDE_PATH) \
	$(THREAD_INCLUDE_PATH) \
	$(UTILITY_INCLUDE_PATH) \
	$(SHARED_NETWORK_INCLUDE_PATH) \
	$(PCRE_INCLUDE_PATH) \
	$(BOOST_INCLUDE_PATH) \
	$(SHARED_REGEX_INCLUDE_PATH) \
	../../../../../../external/3rd/library/regex \
	../../../../../../external/3rd/library/soePlatform/ \
	../../../../../../external/ours/library/singleton/include \
	../../../../../../external/ours/library/unicode/include \
	../../../../../../external/ours/library/unicodeArchive/include \
	../../../../../../game/server/application/SwgGameServer/include/public	\
	../../../../../../game/shared/library/swgSharedNetworkMessages/include/public	\
	../../../../../../game/shared/library/swgSharedUtility/include/public	

debug_custom_step: ../../src/shared/generated/Packager.cpp

release_custom_step: ../../src/shared/generated/Packager.cpp

depend_custom_step: ../../src/shared/generated/Packager.cpp

custom_clean_step:
	rm ../../src/shared/generated/Packager.cpp

../../src/shared/generated/Packager.cpp: ../../src/shared/generated/Packager_cpp.template ../../../codegen/package_data.txt ../../../codegen/make_packages.pl
	cp ../../src/shared/generated/Packager_cpp.template ../../src/shared/generated/Packager.cpp
	chmod +w ../../src/shared/generated/Packager.cpp
	perl ../../../codegen/make_packages.pl --datafile ../../../codegen/package_data.txt --source ../../src/shared/generated/Packager.cpp

include ../../../../../build/shared_unix/library.mak
