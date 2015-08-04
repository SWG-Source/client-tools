LIBRARY = sharedCollision

include ../../../../../build/shared_unix/defs.mak
include ../../../../../build/$(PLATFORM)/platformDefs.mak

INCLUDE_DIRS = \
	../../src/shared \
	../../include/public \
	../../include/private \
	$(COMMON_ENGINE_LIBRARY_INCLUDES) \
	$(FILE_INCLUDE_PATH) \
	$(MATH_INCLUDE_PATH) \
	$(OBJECT_INCLUDE_PATH) \
	$(RANDOM_INCLUDE_PATH) \
	$(SYNCHRONIZATION_INCLUDE_PATH) \
	../../../../../../external/ours/library/archive/include \
	../../../../../../external/ours/library/unicode/include \
	../../../../../../external/ours/library/localization/include \
	../../../../../../external/ours/library/fileInterface/include/public \
	../../../sharedGame/include/public \
	../../../sharedLog/include/public \
	../../../sharedMathArchive/include/public \
	../../../sharedMessageDispatch/include/public \
	../../../sharedTemplate/include/public \
	../../../sharedTemplateDefinition/include/public \
	../../../sharedTerrain/include/public \
	../../../sharedUtility/include/public \
	../../../../../../external/3rd/library/boost

include ../../../../../build/shared_unix/library.mak
