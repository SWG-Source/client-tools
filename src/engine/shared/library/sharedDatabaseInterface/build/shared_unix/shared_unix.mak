LIBRARY = sharedDatabaseInterface

include ../../../../../build/shared_unix/defs.mak
include ../../../../../build/$(PLATFORM)/platformDefs.mak

SHARED_SRC_DIR = ../../../../../shared/library
SERVER_SRC_DIR = ../../..

INCLUDE_DIRS = \
	../../include/public \
	../../include/private \
	../../src/shared/core \
	../../src/shared/queue \
	../../src/$(PLATFORM) \
	$(SHARED_SRC_DIR)/sharedDebug/include/public \
	$(SHARED_SRC_DIR)/sharedFoundation/include/public \
	$(SHARED_SRC_DIR)/sharedFoundationTypes/include/public \
	$(SHARED_SRC_DIR)/sharedMemoryManager/include/public \
	$(SHARED_SRC_DIR)/sharedSynchronization/include/public \
	$(SHARED_SRC_DIR)/sharedThread/include/public \
	$(SHARED_SRC_DIR)/sharedLog/include/public \
	../../../../../../external/3rd/library/stl/stlport \
	../../../../../../external/ours/library/archive/include \
	../../../../../../external/ours/library/unicode/include \
	../../../../../../external/ours/library/unicodeArchive/include/public \
	../../../../../../external/3rd/library/oracle/include


ifdef ORACLE_HOME
	INCLUDE_DIRS += ../../src_oci
	CUSTOM_SRC_LIST =$(shell find $$PWD/../../src_oci -name "*.cpp")
	CUSTOM_CPP_FLAGS = -DDBLIBRARY_OCI
#	CUSTOM_LINK_FLAGS = $(ORACLE_HOME)/lib/libclntsh.so
else
#error ORACLE_HOME not defined
	INCLUDE_DIRS += ../../src_odbc
	CUSTOM_SRC_LIST =$(shell find $$PWD/../../src_odbc -name "*.cpp")
	CUSTOM_CPP_FLAGS = -DDBLIBRARY_ODBC
endif

include ../../../../../build/shared_unix/library.mak
