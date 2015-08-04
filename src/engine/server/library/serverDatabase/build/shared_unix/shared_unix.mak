LIBRARY = serverDatabase

include ../../../../../build/shared_unix/defs.mak
include ../../../../../build/$(PLATFORM)/platformDefs.mak

SHARED_SRC_DIR = ../../../../../shared/library
SERVER_SRC_DIR = ../../..

INCLUDE_DIRS = \
	../../include/public \
	../../include/private \
	../../src/shared \
	../../src/$(PLATFORM) \
	$(SERVER_SRC_DIR)/serverGame/include/public \
	$(SERVER_SRC_DIR)/serverKeyShare/include/public \
	$(SERVER_SRC_DIR)/serverNetworkMessages/include/public \
	$(SERVER_SRC_DIR)/serverUtility/include/public \
	$(SHARED_SRC_DIR)/sharedDatabaseInterface/include/public \
	$(SHARED_SRC_DIR)/sharedDebug/include/public \
	$(SHARED_SRC_DIR)/sharedFile/include/public \
	$(SHARED_SRC_DIR)/sharedFoundation/include/public \
	$(SHARED_SRC_DIR)/sharedFoundationTypes/include/public \
	$(SHARED_SRC_DIR)/sharedLog/include/public \
	$(SHARED_SRC_DIR)/sharedMemoryManager/include/public \
	$(SHARED_SRC_DIR)/sharedMemoryBlockManager/include/public \
	$(SHARED_SRC_DIR)/sharedMath/include/public \
	$(SHARED_SRC_DIR)/sharedMathArchive/include/public \
	$(SHARED_SRC_DIR)/sharedMessageDispatch/include/public \
	$(SHARED_SRC_DIR)/sharedNetworkMessages/include/public \
	$(SHARED_SRC_DIR)/sharedRandom/include/public \
	$(SHARED_SRC_DIR)/sharedObject/include/public \
	$(SHARED_SRC_DIR)/sharedSynchronization/include/public \
	$(SHARED_SRC_DIR)/sharedThread/include/public \
	$(SHARED_SRC_DIR)/sharedUtility/include/public \
	$(SHARED_NETWORK_INCLUDE_PATH) \
	$(COMMON_ENGINE_INCLUDES) \
	../../../../../../external/ours/library/archive/include \
	../../../../../../external/ours/library/singleton/include \
	../../../../../../external/ours/library/vnlv3 \
	../../../../../../external/ours/library/vnlv3/$(PLATFORM) \
	../../../../../../game/shared/library/swgSharedUtility/include/public	

include ../../../../../build/shared_unix/library.mak



#####################
# Special targets   #
#####################
# This isn't ideal, but it will do until we've properly sorted what's in engine and what's in game.

encoder:  ../../../../../../game/server/application/SwgDatabaseServer/src/shared/core/Encoder.cpp ../../../../../../game/server/application/SwgDatabaseServer/src/shared/core/Decoder.cpp

clean_encoder:
	rm ../../../../../../game/server/application/SwgDatabaseServer/src/shared/core/Encoder.cpp
	rm ../../../../../../game/server/application/SwgDatabaseServer/src/shared/core/Decoder.cpp

../../../../../../game/server/application/SwgDatabaseServer/src/shared/core/Encoder.cpp: ../../codegen/makeencoder.pl ../../../serverGame/src/shared/object/*.h ../../../serverGame/src/shared/object/*.cpp
	@echo Generating Encoder.cpp.  If you see a \"permission denied\" error, be sure you have the file checked out.  \(Add one more "../" to the path it reports.\)
	@cd ../../codegen ; perl makeencoder.pl ../../serverGame/src/shared/object/*.h ../../serverGame/src/shared/object/*.cpp > ../../../../../game/server/application/SwgDatabaseServer/src/shared/core/Encoder.cpp

../../../../../../game/server/application/SwgDatabaseServer/src/shared/core/Decoder.cpp: ../../codegen/makedecoder.pl ../../../serverGame/src/shared/object/*.h ../../../serverGame/src/shared/object/*.cpp
	@echo Generating Decoder.cpp.  If you see a \"permission denied\" error, be sure you have the file checked out.  \(Add one more "../" to the path it reports.\)
	@cd ../../codegen ; perl makedecoder.pl ../../serverGame/src/shared/object/*.h ../../serverGame/src/shared/object/*.cpp > ../../../../../game/server/application/SwgDatabaseServer/src/shared/core/Decoder.cpp
