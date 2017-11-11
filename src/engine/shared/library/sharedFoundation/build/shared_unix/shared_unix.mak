LIBRARY = sharedFoundation

include ../../../../../build/shared_unix/defs.mak
include ../../../../../build/$(PLATFORM)/platformDefs.mak


INCLUDE_DIRS = \
	../../include/public \
	../../include/private \
	../../src/shared \
	../../src/$(PLATFORM) \
	$(COMPRESSION_PATH)/include/public \
	$(FILE_PATH)/include/public \
	$(FILE_INTERFACE_INCLUDE_PATH) \
	$(IOWIN_PATH)/include/public \
	$(MATH_PATH)/include/public \
	$(SYNCHRONIZATION_INCLUDE_PATH) \
	$(RANDOM_PATH)/include/public \
	$(THREAD_PATH)/include/public \
	../../../../../../external/ours/library/localization/include \
	../../../../../../external/ours/library/localizationArchive/include/public \
	../../../../../../external/ours/library/unicode/include \
	../../../../../../external/ours/library/unicodeArchive/include \
	../../../sharedMathArchive/include/public \
	../../../../../../external/ours/library/archive/include \
	$(COMMON_ENGINE_LIBRARY_INCLUDES) \

include ../../../../../build/shared_unix/library.mak
