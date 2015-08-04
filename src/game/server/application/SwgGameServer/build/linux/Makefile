include ../../../../../build/linux/platformDefs.mak
include ../../../../../build/shared_unix/defs.mak

OUTPUT = SwgGameServer

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
	../../src/shared/core 		\
	../../src/shared/combat 	\
	../../src/shared/console 	\
	../../src/shared/controller 	\
	../../src/shared/object 	\
	../../src/shared/lint 		\
	../../include/private 		\
	../../include/public 		\
	$(COMMON_ENGINE_INCLUDES)  	\
	$(BOOST_INCLUDE_PATH)		\
	$(COLLISION_INCLUDE_PATH)	\
	$(SHARED_GAME_INCLUDE_PATH)	\
	$(SKILL_SYSTEM_INCLUDE_PATH)	\
	$(SWG_SHARED_UTILITY_INCLUDE_PATH) \
	$(SERVER_GAME_INCLUDE_PATH)	\
	$(SHARED_REGEX_INCLUDE_PATH)	\
	$(TERRAIN_INCLUDE_PATH)		\
	$(SERVER_PATHFINDING_INCLUDE_PATH) \
	$(SERVER_SCRIPT_INCLUDE_PATH)	\
	$(SWG_SHARED_NETWORK_MESSAGES_INCLUDE_PATH)	\
	$(SWG_SERVER_NETWORK_MESSAGES_INCLUDE_PATH) \
	$(COMMAND_PARSER_INCLUDE_PATH)	\

DEBUG_LINK_LIBRARIES =  \
	$(COMMON_ENGINE_DEBUG_LIBS)	 	\
	$(STL_LIB)                              \
	$(ZLIB_LIB) 				\
	$(PCRE_DEBUG_LIB)


DEBUG_SHARED_LIBRARIES= \
	$(COMMON_ENGINE_SHARED_DEBUG_LIBS)	\
	$(SHARED_GAME_DEBUG_LIB)	\
	$(SKILL_SYSTEM_DEBUG_LIB)	\
	$(SWG_SHARED_UTILITY_DEBUG_LIB) \
	$(SERVER_GAME_DEBUG_LIB)	\
	$(SHARED_REGEX_DEBUG_LIB)	\
	$(TERRAIN_DEBUG_LIB)		\
	$(SERVER_PATHFINDING_DEBUG_LIB) \
	$(PATHFINDING_DEBUG_LIB) \
	$(SERVER_SCRIPT_DEBUG_LIB)	\
	$(SWG_SHARED_NETWORK_MESSAGES_DEBUG_LIB)	\
	$(SWG_SERVER_NETWORK_MESSAGES_DEBUG_LIB) \
	$(COMMAND_PARSER_DEBUG_LIB)	\


#	$(SERVER_GAME_DEBUG_LIB)

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

#######################################################
#  include the common makefile
#######################################################

include ../../../../../build/shared_unix/application.mak
