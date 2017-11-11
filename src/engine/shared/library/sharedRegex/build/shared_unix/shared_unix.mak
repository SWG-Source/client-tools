LIBRARY = sharedRegex

include ../../../../../build/shared_unix/defs.mak
include ../../../../../build/$(PLATFORM)/platformDefs.mak

INCLUDE_DIRS = \
	../../include/public \
	../../include/private \
	../../src/shared \
	../../src/$(PLATFORM) \
	$(PCRE_INCLUDE_PATH) \
	$(COMMON_ENGINE_LIBRARY_INCLUDES)

CUSTOM_CPP_FLAGS = -DPCRE_STATIC

include ../../../../../build/shared_unix/library.mak
