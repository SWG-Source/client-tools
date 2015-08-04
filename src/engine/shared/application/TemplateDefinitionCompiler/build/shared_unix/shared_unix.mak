DEBUG_PLATFORM_CPP_FLAGS	=-DLINUX -D_REENTRANT -D_XOPEN_SOUCE=500 -D_GNU_SOURCE -DPCRE_STATIC $(PERSONAL_FLAGS)
RELEASE_PLATFORM_CPP_FLAGS	=-DLINUX -D_REENTRANT -D_XOPEN_SOUCE=500 -D_GNU_SOURCE -DPCRE_STATIC $(PERSONAL_FLAGS)

include ../../../../../build/linux/platformDefs.mak
include ../../../../../build/shared_unix/defs.mak

OUTPUT = TemplateDefinitionCompiler

#######################################################
#  standard defines
#######################################################

DEBUG_LINK_FLAGS    +=-lpthread -lz -lodbc
RELEASE_LINK_FLAGS  +=-lpthread -lz -lodbc

#######################################################
#  includes & linked libraries
#######################################################

INCLUDE_DIRS = \
	../../src/linux \
	../../src/shared/core \
	../../../../../shared/library/sharedTemplateDefinition/include/public \
	../../../../../../external/ours/library/localization/include \
	$(SHARED_REGEX_INCLUDE_PATH) \
	$(PCRE_INCLUDE_PATH) \
	$(COMMON_ENGINE_INCLUDES)	

DEBUG_LINK_LIBRARIES = \
	$(ARCHIVE_DEBUG_LIB) \
	$(UNICODE_DEBUG_LIB) \
	$(UNICODE_ARCHIVE_DEBUG_LIB) \
	$(LOCALIZATION_DEBUG_LIB) \
	$(LOCALIZATION_ARCHIVE_DEBUG_LIB) \
	$(COMPRESSION_DEBUG_LIB) \
	$(DEBUG_DEBUG_LIB) \
	$(FILE_DEBUG_LIB) \
	$(FILE_INTERFACE_DEBUG_LIB) \
	$(FOUNDATION_DEBUG_LIB) \
	$(MATH_DEBUG_LIB) \
	$(MEMORY_MANAGER_DEBUG_LIB) \
	$(RANDOM_DEBUG_LIB) \
	$(SYNCHRONIZATION_DEBUG_LIB) \
	$(THREAD_DEBUG_LIB) \
	$(IOWIN_DEBUG_LIB) \
	$(UTILITY_DEBUG_LIB) \
	../../../../../../compile/linux/sharedTemplateDefinition/debug/libsharedTemplateDefinition_d.a \
	$(SHARED_REGEX_DEBUG_LIB) \
	$(PCRE_DEBUG_LIB) \
	$(STL_LIB) \

DEBUG_SHARED_LIBRARIES= \
	$(COMMON_ENGINE_SHARED_DEBUG_LIBS)

RELEASE_LINK_LIBRARIES = \
	$(ARCHIVE_RELEASE_LIB) \
	$(UNICODE_RELEASE_LIB) \
	$(UNICODE_ARCHIVE_RELEASE_LIB) \
	$(LOCALIZATION_RELEASE_LIB) \
	$(LOCALIZATION_ARCHIVE_RELEASE_LIB) \
	$(COMPRESSION_RELEASE_LIB) \
	$(DEBUG_RELEASE_LIB) \
	$(FILE_RELEASE_LIB) \
	$(FILE_INTERFACE_RELEASE_LIB) \
	$(FOUNDATION_RELEASE_LIB) \
	$(MATH_RELEASE_LIB) \
	$(MEMORY_MANAGER_RELEASE_LIB) \
	$(RANDOM_RELEASE_LIB) \
	$(SYNCHRONIZATION_RELEASE_LIB) \
	$(THREAD_RELEASE_LIB) \
	$(IOWIN_RELEASE_LIB) \
	$(UTILITY_RELEASE_LIB) \
	../../../../../../compile/linux/sharedTemplateDefinition/debug/libsharedTemplateDefinition_r.a \
	$(SHARED_REGEX_RELEASE_LIB) \
	$(PCRE_RELEASE_LIB) \
	$(STL_LIB) \

RELEASE_SHARED_LIBRARIES= \
	$(COMMON_ENGINE_SHARED_RELEASE_LIBS)

#######################################################
#  Project depends
#  (The makefile for these projects will be
#  automatically invoked.)
#######################################################

DEBUG_PROJECT_DEPENDS = \

RELEASE_PROJECT_DEPENDS = \

#######################################################
#  Source file list
#######################################################

SHARED_SRC_LIST 	= \
	../../src/shared/core/TemplateDefinitionCompiler.cpp
PLATFORM_SRC_LIST 	= \
	../../src/linux/FirstTemplateDefinitionCompiler.cpp
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
