include ../../../../../build/linux/platformDefs.mak
include ../../../../../build/shared_unix/defs.mak

OUTPUT = Phony

#######################################################
#  standard defines
#######################################################

DEBUG_LINK_FLAGS    +=-lpthread -ldl
RELEASE_LINK_FLAGS  +=-lpthread -ldl

#######################################################
#  includes & linked libraries
#######################################################

INCLUDE_DIRS = 				\
	../../src/linux 		\
	../../src/shared 		\
	$(COMMON_ENGINE_INCLUDES)  	\
	$(SHARED_GAME_INCLUDE_PATH)	\
	$(SKILL_SYSTEM_INCLUDE_PATH)	\
	$(SWG_SHARED_UTILITY_INCLUDE_PATH) \
	$(SERVER_GAME_INCLUDE_PATH)

DEBUG_LINK_LIBRARIES =  \
	$(COMMON_ENGINE_DEBUG_LIBS)	 	\
	$(STL_LIB)                              \
	$(ZLIB_LIB)

DEBUG_SHARED_LIBRARIES= \
	$(COMMON_ENGINE_SHARED_DEBUG_LIBS)	\
	$(SHARED_GAME_DEBUG_LIB)		\
	$(SKILL_SYSTEM_DEBUG_LIB)		\
	$(SWG_SHARED_UTILITY_DEBUG_LIB)		\

#	$(SERVER_GAME_DEBUG_LIB)

#######################################################
#  Source file list
#######################################################

#SHARED_SRC_LIST 	=$(shell find $$PWD/../../src/shared -name "*.cpp")
PLATFORM_SRC_LIST 	=$(shell find $$PWD/../../src/linux -name "*.cpp")
SRC_LIST 		=$(SHARED_SRC_LIST) $(PLATFORM_SRC_LIST)


#######################################################
#  Custom build steps
#######################################################

debug_custom_step:

release_custom_step:

#######################################################
#  include the common makefile
#######################################################

include ../../../../../build/shared_unix/application.mak
