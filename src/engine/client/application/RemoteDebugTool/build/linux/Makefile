DEBUG_PLATFORM_CPP_FLAGS	=-DLINUX -D_REENTRANT -D_XOPEN_SOURCE=500 -D_GNU_SOURCE -DTRUE=TRUE $(PERSONAL_FLAGS)
DEBUG_PLATFORM_CPP_FLAGS	=-DLINUX -D_REENTRANT -D_XOPEN_SOURCE=500 -D_GNU_SOURCE -DTRUE=TRUE $(PERSONAL_FLAGS)

include ../../../../../build/linux/platformDefs.mak
include ../../../../../build/shared_unix/defs.mak

OUTPUT	= remoteDebugTool

DEBUG_LINK_FLAGS += -lpthread -lodbc -L/usr/lib/qt-3.0.3/lib -lqui -ldl
RELEASE_LINK_FLAGS += -lpthread -lodbc -L/usr/lib/qt-3.0.3/lib -lqui -ldl

INCLUDE_DIRS = \
	../../src/shared \
	../../include/private \
	../../include/public \
	$(ARCHIVE_INCLUDE_PATH) \
	$(DEBUG_INCLUDE_PATH) \
	$(FILE_INCLUDE_PATH) \
	$(FOUNDATION_INCLUDE_PATH) \
	$(FOUNDATION_TYPES_INCLUDE_PATH) \
	$(SYNCHRONIZATION_INCLUDE_PATH) \
	$(THREAD_INCLUDE_PATH) \
	$(UTILITY_INCLUDE_PATH) \
	$(SHARED_NETWORK_INCLUDE_PATH) \
	$(CLIENT_REMOTE_DEBUG_INCLUDE_PATH) \
	$(STL_INCLUDE_PATH) \
	$(COMMON_ENGINE_LIBRARY_INCLUDES) \
	/usr/lib/qt-3.0.3/include \

DEBUG_LINK_LIBRARIES = \
	$(CLIENT_REMOTE_DEBUG_DEBUG_LIB) \
	$(COMMON_ENGINE_DEBUG_LIBS) \
	$(UNICODE_DEBUG_LIB)

DEBUG_SHARED_LIBRARIES = \

RELEASE_LINK_LIBRARIES = \
	$(CLIENT_REMOTE_DEBUG_RELEASE_LIB) \
	$(COMMON_ENGINE_RELEASE_LIBS) \
	$(UNICODE_RELEASE_LIB)

RELEASE_SHARED_LIBRARIES = \

SHARED_SRC_LIST = $(shell find $$PWD/../../src/shared -name "*.cpp")
PLATFORM_SRC_LIST =
SRC_LIST = $(SHARED_SRC_LIST) $(PLATFORM_SRC_LIST)

debug_custom_step:
release_custom_step:
depend_custom_step:

%.moc: %.h
	moc -o $@ $<

include ../../../../../build/shared_unix/application.mak


