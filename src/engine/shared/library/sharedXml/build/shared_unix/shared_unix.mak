LIBRARY = sharedXml

include ../../../../../build/shared_unix/defs.mak
include ../../../../../build/$(PLATFORM)/platformDefs.mak

INCLUDE_DIRS = \
	../../include/public \
	../../include/private \
	../../src/shared \
	../../src/$(PLATFORM) \
	$(FILE_PATH)/include/public \
	$(FILE_INTERFACE_PATH)/include/public \
	$(MATH_PATH)/include/public \
	$(MATH_ARCHIVE_INCLUDE_PATH) \
	$(RANDOM_PATH)/include/public \
	$(LOCALIZATION_INCLUDE_PATH) \
	$(SYNCHRONIZATION_PATH)/include/public \
	../../../../../../external/3rd/library/boost \
	../../../../../../external/ours/library/unicode/include \
	../../../../../../external/ours/library/archive/include \
	$(COMMON_ENGINE_LIBRARY_INCLUDES)	

include ../../../../../build/shared_unix/library.mak
