DEBUG_PLATFORM_CPP_FLAGS	=-DLINUX -D_REENTRANT -D_XOPEN_SOUCE=500 -D_GNU_SOURCE $(PERSONAL_FLAGS)
RELEASE_PLATFORM_CPP_FLAGS	=-DLINUX -D_REENTRANT -D_XOPEN_SOUCE=500 -D_GNU_SOURCE $(PERSONAL_FLAGS)

include ../../../../../build/linux/platformDefs.mak
include ../../../../../build/shared_unix/defs.mak


OUTPUT = Md5sum

#######################################################
#  overrides of standard defines
#######################################################
	DEBUG_LINK_FLAGS    +=-lpthread -lc -ldl
	RELEASE_LINK_FLAGS  +=-lpthread -ldl

#######################################################
#  includes & linked libraries
#######################################################

INCLUDE_DIRS = \
	../../src/shared \
	../../include/private \
	../../include/public \
	$(DEBUG_INCLUDE_PATH) \
	$(FOUNDATION_INCLUDE_PATH) \
	$(FOUNDATION_TYPES_INCLUDE_PATH) \
	$(SYNCHRONIZATION_INCLUDE_PATH) \
	$(THREAD_INCLUDE_PATH) \
	$(STL_INCLUDE_PATH) \
	$(COMMON_ENGINE_LIBRARY_INCLUDES) \
	../../../../../../external/ours/library/singleton/include

DEBUG_LINK_LIBRARIES = \
	$(ARCHIVE_DEBUG_LIB) \
	$(DEBUG_DEBUG_LIB) \
	$(FOUNDATION_DEBUG_LIB) \
	$(LOCALIZATION_DEBUG_LIB) \
	$(LOCALIZATION_ARCHIVE_DEBUG_LIB) \
	$(MEMORY_MANAGER_DEBUG_LIB) \
	$(SYNCHRONIZATION_DEBUG_LIB) \
	$(THREAD_DEBUG_LIB) \
	$(UNICODE_DEBUG_LIB) \
	$(UNICODE_ARCHIVE_DEBUG_LIB) \
	$(STL_LIB)

DEBUG_SHARED_LIBRARIES= \

RELEASE_LINK_LIBRARIES = \
	$(ARCHIVE_RELEASE_LIB) \
	$(DEBUG_RELEASE_LIB) \
	$(FOUNDATION_RELEASE_LIB) \
	$(LOCALIZATION_RELEASE_LIB) \
	$(LOCALIZATION_ARCHIVE_RELEASE_LIB) \
	$(MEMORY_MANAGER_RELEASE_LIB) \
	$(SYNCHRONIZATION_RELEASE_LIB) \
	$(THREAD_RELEASE_LIB) \
	$(UNICODE_RELEASE_LIB) \
	$(UNICODE_ARCHIVE_RELEASE_LIB) \
	$(STL_LIB)

RELEASE_SHARED_LIBRARIES= \


#######################################################
#  Project depends
#  (The makefile for these projects will be
#  automatically invoked.)
#######################################################


#######################################################
#  Source file list
#######################################################

SHARED_SRC_LIST 	=$(shell find $$PWD/../../src/shared -name "*.cpp")
PLATFORM_SRC_LIST 	=
SRC_LIST 		=$(SHARED_SRC_LIST) $(PLATFORM_SRC_LIST)

#######################################################
#  Custom build steps
#######################################################

debug_custom_step: 

release_custom_step:

depend_custom_step: 

#######################################################
#  include the common makefile
#######################################################

include ../../../../../build/shared_unix/application.mak
