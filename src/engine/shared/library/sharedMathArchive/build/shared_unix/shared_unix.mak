LIBRARY = mathArchive

include ../../../build/shared_unix/defs.mak
include ../../../build/$(PLATFORM)/platformDefs.mak


INCLUDE_DIRS = \
	../../include/public \
	../../include/private \
	../../src/shared \
	../../src/$(PLATFORM) \
	../../../../../../external/3rd/library/stl/stlport 

include ../../../build/shared_unix/common.mak
