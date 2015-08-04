DEBUG_PLATFORM_CPP_FLAGS	=-DLINUX -D_REENTRANT -D_XOPEN_SOUCE=500 -D_GNU_SOURCE $(PERSONAL_FLAGS)
RELEASE_PLATFORM_CPP_FLAGS	=-DLINUX -D_REENTRANT -D_XOPEN_SOUCE=500 -D_GNU_SOURCE $(PERSONAL_FLAGS)

include ../../../../../build/linux/platformDefs.mak
include ../../../../../build/shared_unix/defs.mak


OUTPUT = CustomerServiceServer

#######################################################
#  standard defines
#######################################################

DEBUG_LINK_FLAGS    +=-lpthread -ldl -lxml2
RELEASE_LINK_FLAGS  +=-lpthread -ldl -lxml2

#######################################################
#  includes & linked libraries
#######################################################

INCLUDE_DIRS = \
	../../src/linux \
	../../src/shared \
	$(COMMON_ENGINE_INCLUDES) \
	$(SHARED_NETWORK_MESSAGES) \
	../../../../../../external/3rd/library/libxml/include \
	../../../../../../external/3rd/library/soePlatform \
	../../../../../../external/3rd/library/soePlatform/CSAssist/utils \
	../../../../../../external/3rd/library/soePlatform/CSAssist/projects/CSAssist/CSAssistgameapi \
	/usr/include/libxml2

DEBUG_LINK_LIBRARIES = \
	$(COMMON_ENGINE_DEBUG_LIBS) \
	../../../../../../external/3rd/library/soePlatform/CSAssist/projects/CSAssist/CSAssistgameapi/debug/libCSAssistgameapi.a \
        $(STL_LIB)                      \
        $(ZLIB_LIB)                     \

DEBUG_SHARED_LIBRARIES= \
	$(COMMON_ENGINE_SHARED_DEBUG_LIBS)	\
	$(SHARED_GAME_DEBUG_LIB)		\
	$(SKILL_SYSTEM_DEBUG_LIB)		\
	$(SWG_SHARED_UTILITY_DEBUG_LIB)		\

RELEASE_LINK_LIBRARIES = \
	$(COMMON_ENGINE_RELEASE_LIBS) \
	../../../../../../external/3rd/library/soePlatform/CSAssist/projects/CSAssist/CSAssistgameapi/release/libCSAssistgameapi.a \
	../../../../../../external/3rd/library/soePlatform/CSAssist/lib/release/libBase_MT.a


RELEASE_SHARED_LIBRARIES= \
	$(COMMON_ENGINE_SHARED_RELEASE_LIBS) \
	xml2

../../../../../../external/3rd/library/soePlatform/CSAssist/projects/CSAssist/CSAssistgameapi/debug/libCSAssistgameapi.a:
	make -C ../../../../../../external/3rd/library/soePlatform/CSAssist/projects/CSAssist/CSAssistgameapi debug

../../../../../../external/3rd/library/soePlatform/CSAssist/projects/CSAssist/CSAssistgameapi/release/libCSAssistgameapi.a:
	make -C ../../../../../../external/3rd/library/soePlatform/CSAssist/projects/CSAssist/CSAssistgameapi release

../../../../../../external/3rd/library/soePlatform/CSAssist/lib/release/libUnicode.a:
	make -C ../../../../../../external/3rd/library/soePlatform/CSAssist/utils/Unicode

../../../../../../external/3rd/library/soePlatform/CSAssist/lib/release/libBase_MT.a:
	make -C ../../../../../../external/3rd/library/soePlatform/CSAssist/utils/Base/linux release_mt

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

#debug_custom_step:
#	cp -f -u ../../../../../../external/3rd/library/soePlatform/libs/Linux-Debug/*.so ../../../../../../../dev/linux

#release_custom_step:
#	cp -f -u ../../../../../../external/3rd/library/soePlatform/libs/Linux-Release/*.so ../../../../../../../dev/linux

depend_custom_step:

#######################################################
#  include the common makefile
#######################################################

include ../../../../../build/shared_unix/application.mak
