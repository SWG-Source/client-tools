LIBRARY = serverNetworkMessages

include ../../../../../build/shared_unix/defs.mak
include ../../../../../build/$(PLATFORM)/platformDefs.mak

SHARED_SRC_DIR = ../../../../../shared/library
SERVER_SRC_DIR = ../../..

INCLUDE_DIRS = \
	$(COMMON_ENGINE_INCLUDES) \
	../../include/public \
	../../include/private \
	$(GAME_INCLUDE_PATH) \
	$(SERVER_GAME_INCLUDE_PATH) \
	$(SHARED_GAME_INCLUDE_PATH) \
	$(SWG_SHARED_UTILITY_INCLUDE_PATH) \
	$(BOOST_INCLUDE_PATH)

include ../../../../../build/shared_unix/library.mak
