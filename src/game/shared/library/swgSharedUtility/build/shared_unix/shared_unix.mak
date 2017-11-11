LIBRARY = swgSharedUtility

SRC_DIR = ../../../../../..

include $(SRC_DIR)/game/build/shared_unix/defs.mak
include $(SRC_DIR)/game/build/$(PLATFORM)/platformDefs.mak

SHARED_SRC_DIR = $(SRC_DIR)/engine/shared/library
SERVER_SRC_DIR = $(SRC_DIR)/engine/server/library
EXTERNAL_SRC_DIR = $(SRC_DIR)/external
SWG_SERVER_SRC_DIR = $(SRC_DIR)/game/server/application

INCLUDE_DIRS = \
	../../include/public \
	../../include/private \
	../../../swgSharedUtility/include/public \
	$(SERVER_SRC_DIR)/serverKeyShare/include/public \
	$(SERVER_SRC_DIR)/serverGame/include/public \
	$(SHARED_SRC_DIR)/sharedDebug/include/public \
	$(SHARED_SRC_DIR)/sharedFile/include/public \
	$(SHARED_SRC_DIR)/sharedGame/include/public \
	$(SHARED_SRC_DIR)/sharedFoundation/include/public \
	$(SHARED_SRC_DIR)/sharedFoundationTypes/include/public \
	$(SHARED_SRC_DIR)/sharedMemoryManager/include/public \
	$(SHARED_SRC_DIR)/sharedMath/include/public \
	$(SHARED_SRC_DIR)/sharedMathArchive/include/public \
	$(SHARED_SRC_DIR)/sharedMessageDispatch/include/public \
	$(SHARED_SRC_DIR)/sharedNetworkMessages/include/public \
	$(SHARED_SRC_DIR)/sharedObject/include/public \
	$(SHARED_SRC_DIR)/sharedRandom/include/public \
	$(SHARED_SRC_DIR)/sharedUtility/include/public \
	$(SWG_SERVER_SRC_DIR)/SwgGameServer/include/public \
	$(EXTERNAL_SRC_DIR)/3rd/library/stlport453/stlport \
	$(EXTERNAL_SRC_DIR)/ours/library/archive/include \
	$(EXTERNAL_SRC_DIR)/ours/library/unicode/include \
	$(EXTERNAL_SRC_DIR)/ours/library/localization/include \
	$(EXTERNAL_SRC_DIR)/ours/library/unicodeArchive/include \
	$(EXTERNAL_SRC_DIR)/ours/library/vnlv3 \
	$(EXTERNAL_SRC_DIR)/ours/library/vnlv3/$(PLATFORM)

include $(SRC_DIR)/game/build/shared_unix/library.mak
