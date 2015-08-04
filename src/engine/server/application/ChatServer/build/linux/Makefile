DEBUG_PLATFORM_CPP_FLAGS	=-DLINUX -D_REENTRANT -D_XOPEN_SOUCE=500 -D_GNU_SOURCE -DEXTERNAL_DISTRO -DNAMESPACE="ChatSystem" $(PERSONAL_FLAGS)
RELEASE_PLATFORM_CPP_FLAGS	=-DLINUX -D_REENTRANT -D_XOPEN_SOUCE=500 -D_GNU_SOURCE -DEXTERNAL_DISTRO -DNAMESPACE="ChatSystem" $(PERSONAL_FLAGS)

include ../../../../../build/linux/platformDefs.mak
include ../../../../../build/shared_unix/defs.mak


OUTPUT = ChatServer

#######################################################
#  standard defines
#######################################################

#DEBUG_LINK_FLAGS    +=-lpthread -ldl -L../../../../../../external/3rd/library/soePlatform/libs/Linux-Debug
#RELEASE_LINK_FLAGS  +=-lpthread -ldl -L../../../../../../external/3rd/library/soePlatform/libs/Linux-Release
DEBUG_LINK_FLAGS    +=-lpthread -ldl -L../../../../../../external/3rd/library/soePlatform/ChatAPI2/ChatAPI/lib/debug
RELEASE_LINK_FLAGS  +=-lpthread -ldl -L../../../../../../external/3rd/library/soePlatform/ChatAPI2/ChatAPI/lib/release

#######################################################
#  includes & linked libraries
#######################################################

INCLUDE_DIRS = \
	../../src/linux \
	../../src/shared \
	$(COMMON_ENGINE_INCLUDES) \
	$(SHARED_NETWORK_MESSAGES) \
	../../../../../../external/3rd/library/soePlatform/ChatAPI2/ChatAPI/utils/ \
	../../../../../../external/3rd/library/soePlatform/ChatAPI2/ChatAPI/projects/Chat

DEBUG_LINK_LIBRARIES = \
	$(CHATAPI_DEBUG_LIB) 		\
	$(COMMON_ENGINE_DEBUG_LIBS)	\
        $(STL_LIB)                      \
        $(ZLIB_LIB)                     

DEBUG_SHARED_LIBRARIES= \
	$(COMMON_ENGINE_SHARED_DEBUG_LIBS)	\
	$(SHARED_GAME_DEBUG_LIB)		\
	$(SKILL_SYSTEM_DEBUG_LIB)		\
	$(SWG_SHARED_UTILITY_DEBUG_LIB)		\

RELEASE_LINK_LIBRARIES = \
	../../../../../../external/3rd/library/soePlatform/ChatAPI2/ChatAPI/lib/release/libChatAPI.a \
	$(COMMON_ENGINE_RELEASE_LIBS)

../../../../../../external/3rd/library/soePlatform/ChatAPI2/ChatAPI/lib/release/libChatAPI.a:
	make -C ../../../../../../external/3rd/library/soePlatform/ChatAPI2/ChatAPI/projects/Chat/ChatMono release

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

# debug_custom_step:
#	cp -f -u ../../../../../../external/3rd/library/soePlatform/libs/Linux-Debug/*.so ../../../../../../../dev/linux

# release_custom_step:
# 	cp -f -u ../../../../../../external/3rd/library/soePlatform/libs/Linux-Release/*.so ../../../../../../../dev/linux

depend_custom_step:

#######################################################
#  include the common makefile
#######################################################

include ../../../../../build/shared_unix/application.mak
