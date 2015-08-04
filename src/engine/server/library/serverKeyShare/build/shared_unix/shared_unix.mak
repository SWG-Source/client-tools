LIBRARY = serverKeyShare

include ../../../../../build/shared_unix/defs.mak
include ../../../../../build/$(PLATFORM)/platformDefs.mak

SHARED_SRC_DIR = ../../../../../shared/library
SERVER_SRC_DIR = ../../..

INCLUDE_DIRS = \
	../../include/public \
	../../include/private \
	../../src/shared/ \
	../../src/$(PLATFORM) \
	$(SHARED_SRC_DIR)/sharedDebug/include/public \
	$(SHARED_SRC_DIR)/sharedFoundation/include/public \
	$(SHARED_SRC_DIR)/sharedFoundationTypes/include/public \
	$(SHARED_SRC_DIR)/sharedMemoryManager/include/public \
	$(SHARED_SRC_DIR)/sharedRandom/include/public \
	../../../../../../external/3rd/library/stl/stlport \
	../../../../../../external/ours/library/archive/include \
	../../../../../../external/ours/library/crypto/include \

include ../../../../../build/shared_unix/library.mak
