LIBRARY = sharedSkillSystem

include ../../../../../build/shared_unix/defs.mak
include ../../../../../build/$(PLATFORM)/platformDefs.mak

INCLUDE_DIRS = \
	../../include/public \
	../../include/private \
	../../src/shared \
	$(TERRAIN_INCLUDE_PATH) \
	$(FILE_INCLUDE_PATH) \
	$(FILE_INTERFACE_INCLUDE_PATH) \
	$(MATH_INCLUDE_PATH) \
	$(MATH_ARCHIVE_INCLUDE_PATH) \
	$(MESSAGE_DISPATCH_INCLUDE_PATH) \
	$(NETWORK_MESSAGES_INCLUDE_PATH) \
	$(RANDOM_INCLUDE_PATH) \
	$(SYNCHRONIZATION_INCLUDE_PATH) \
	$(UTILITY_INCLUDE_PATH) \
	../../../../../../external/3rd/library/boost \
	../../../../../../external/ours/library/archive/include \
	../../../../../../external/ours/library/singleton/include \
	../../../../../../external/ours/library/unicode/include \
	$(COMMON_ENGINE_LIBRARY_INCLUDES)

include ../../../../../build/shared_unix/library.mak
