DEBUG_PLATFORM_CPP_FLAGS	=-DLINUX -D_REENTRANT -D_XOPEN_SOUCE=500 -D_GNU_SOURCE $(PERSONAL_FLAGS)
RELEASE_PLATFORM_CPP_FLAGS	=-DLINUX -D_REENTRANT -D_XOPEN_SOUCE=500 -D_GNU_SOURCE $(PERSONAL_FLAGS)

include ../../../../../build/linux/platformDefs.mak
include ../../../../../build/shared_unix/defs.mak


OUTPUT = LogServer

#######################################################
#  standard defines
#######################################################

DEBUG_LINK_FLAGS    +=-lpthread -ldl
RELEASE_LINK_FLAGS  +=-lpthread -ldl

#######################################################
#  includes & linked libraries
#######################################################

INCLUDE_DIRS = \
	../../src/linux \
	../../src/shared \
	$(COMMON_ENGINE_INCLUDES) \
	../../../../../../external/3rd/library/udplibrary

DEBUG_LINK_LIBRARIES = \
	$(COMMON_ENGINE_DEBUG_LIBS)	\
        $(STL_LIB)                      \
        $(ZLIB_LIB)                     \

DEBUG_SHARED_LIBRARIES= \
	$(COMMON_ENGINE_SHARED_DEBUG_LIBS)	\
	$(SHARED_GAME_DEBUG_LIB)		\
	$(SKILL_SYSTEM_DEBUG_LIB)		\
	$(SWG_SHARED_UTILITY_DEBUG_LIB)		\

RELEASE_LINK_LIBRARIES = \
	$(COMMON_ENGINE_RELEASE_LIBS)

RELEASE_SHARED_LIBRARIES= \
	$(COMMON_ENGINE_SHARED_RELEASE_LIBS)

#######################################################
#  Project depends
#  (The makefile for these projects will be
#  automatically invoked.)
#######################################################

DEBUG_PROJECT_DEPENDS = \
	$(COMMON_ENGINE_DEBUG_DEPENDS)

RELEASE_PROJECT_DEPENDS = \
	$(COMMON_ENGINE_RELEASE_DEPENDS)

#######################################################
#  Source file list
#######################################################

SHARED_SRC_LIST 	=$(shell find $$PWD/../../src/shared -name "*.cpp")
PLATFORM_SRC_LIST 	=$(shell find $$PWD/../../src/linux -name "*.cpp")
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
