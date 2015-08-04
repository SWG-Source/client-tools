LIBRARY = crypto

include ../../../../../../engine/build/shared_unix/defs.mak
include ../../../../../../engine/build/linux/platformDefs.mak

SHARED_SRC_DIR = ../../../../../shared/library
SERVER_SRC_DIR = ../../..

INCLUDE_DIRS = \
	../../src/shared/core 		\
	../../src/shared/original 	\
	../../src/shared/wrapper	\
	../../include			\
	$(STL_INCLUDE_PATH) 		\

W_ERROR=-Wall
# TODO this is cheesy.  We are just overriding the library.mak -Werror option since this lib
# has compile warnings.  It actually has quite a few of them, and we should fix them.

include ../../../../../../engine/build/shared_unix/library.mak