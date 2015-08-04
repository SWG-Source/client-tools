LIBRARY = swgSharedNetworkMessages

SRC_DIR = ../../../../../..

include $(SRC_DIR)/game/build/shared_unix/defs.mak
include $(SRC_DIR)/game/build/$(PLATFORM)/platformDefs.mak

SHARED_SRC_DIR = $(SRC_DIR)/engine/shared/library
SERVER_SRC_DIR = $(SRC_DIR)/engine/server/library
EXTERNAL_SRC_DIR = $(SRC_DIR)/external
SWG_SERVER_SRC_DIR = $(SRC_DIR)/game/server/application

INCLUDE_DIRS = \
	$(COMMON_ENGINE_INCLUDES) \
	../../include/public \
	../../include/private \
	../../../swgSharedUtility/include/public \
	$(GAME_INCLUDE_PATH) \
	$(SWG_SERVER_SRC_DIR)/SwgGameServer/include/public \
	$(SHARED_GAME_INCLUDE_PATH)		\
	$(EXTERNAL_SRC_DIR)/ours/library/vnlv3 \
	$(EXTERNAL_SRC_DIR)/ours/library/vnlv3/$(PLATFORM)

include $(SRC_DIR)/game/build/shared_unix/library.mak
