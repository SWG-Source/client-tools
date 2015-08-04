DEBUG_PLATFORM_CPP_FLAGS	=-DLINUX -D_REENTRANT -D_XOPEN_SOUCE=500 -D_GNU_SOURCE $(PERSONAL_FLAGS)
RELEASE_PLATFORM_CPP_FLAGS	=-DLINUX -D_REENTRANT -D_XOPEN_SOUCE=500 -D_GNU_SOURCE $(PERSONAL_FLAGS)

include ../../../../../build/linux/platformDefs.mak
include ../../../../../build/shared_unix/defs.mak


OUTPUT = SwgDatabaseServer

#######################################################
#  standard defines
#######################################################

DEBUG_LINK_FLAGS    +=-lpthread -ldl
RELEASE_LINK_FLAGS  +=-lpthread -ldl

#######################################################
#  includes & linked libraries
#######################################################

INCLUDE_DIRS = \
	../../src/linux \
	../../src/shared \
	../../src/shared/core \
	../../src/shared/buffers \
	../../include/public		\
	$(SHARED_DATABASE_INTERFACE_INCLUDE_PATH) \
	$(SERVER_DATABASE_INCLUDE_PATH)	\
	$(COMMON_ENGINE_INCLUDES)	\
	$(SERVER_GAME_INCLUDE_PATH)	\
	$(SWG_SHARED_UTILITY_INCLUDE_PATH) \
	$(SWG_SHARED_NETWORK_MESSAGES_INCLUDE_PATH) \
	$(SWG_SERVER_NETWORK_MESSAGES_INCLUDE_PATH) \
	$(SHARED_GAME_INCLUDE_PATH)

DEBUG_LINK_LIBRARIES = \
        $(COMMON_ENGINE_DEBUG_LIBS)     \
        $(STL_LIB)                      \
        $(ZLIB_LIB)                     \
	$(PCRE_DEBUG_LIB)

DEBUG_SHARED_LIBRARIES= \
	$(COMMAND_PARSER_DEBUG_LIB) 	\
	$(SERVER_DATABASE_DEBUG_LIB)	\
	$(SHARED_DATABASE_INTERFACE_DEBUG_LIB) \
	$(ORACLE_CLIENT_LIB) \
	$(COMMON_ENGINE_SHARED_DEBUG_LIBS) \
	$(SHARED_GAME_DEBUG_LIB)	\
	$(SKILL_SYSTEM_DEBUG_LIB)	\
	$(SWG_SHARED_UTILITY_DEBUG_LIB) \
	$(SHARED_REGEX_DEBUG_LIB)	\
	$(TERRAIN_DEBUG_LIB)		\
	$(SWG_SHARED_NETWORK_MESSAGES_DEBUG_LIB)	\
	$(SWG_SERVER_NETWORK_MESSAGES_DEBUG_LIB) \
	$(COMMAND_PARSER_DEBUG_LIB)	\
	$(SWG_SHARED_UTILITY_DEBUG_LIB)	\
	$(SHARED_GAME_DEBUG_LIB)	\
	$(SWG_SHARED_NETWORK_MESSAGES_DEBUG_LIB) \
	$(SWG_SERVER_NETWORK_MESSAGES_DEBUG_LIB)

#######################################################
#  Source file list
#######################################################

SHARED_SRC_LIST 	=$(shell find $$PWD/../../src/shared -name "*.cpp")
PLATFORM_SRC_LIST 	=$(shell find $$PWD/../../src/linux -name "*.cpp")
SRC_LIST 		=$(SHARED_SRC_LIST) $(PLATFORM_SRC_LIST)

#######################################################
#  Custom build steps
#######################################################

debug_custom_step: ../../src/shared/generated/Schema.h ../../src/shared/generated/Encoder.cpp ../../src/shared/generated/Decoder.cpp ../../src/shared/generated/LoadObject.cpp ../../src/shared/generated/ObjectQueries.cpp ../../src/shared/generated/ObjectQueries.h

depend_custom_step:  ../../src/shared/generated/Schema.h ../../src/shared/generated/Encoder.cpp ../../src/shared/generated/Decoder.cpp ../../src/shared/generated/LoadObject.cpp ../../src/shared/generated/ObjectQueries.cpp ../../src/shared/generated/ObjectQueries.h


../../src/shared/generated/Schema.h: ../../src/shared/generated/Schema_h.template ../../../../../../engine/server/library/codegen/make_schema_h.pl ../../../../database/schema/*.tab
	cp ../../src/shared/generated/Schema_h.template ../../src/shared/generated/Schema.h
	chmod +w ../../src/shared/generated/Schema.h
	perl ../../../../../../engine/server/library/codegen/make_schema_h.pl --ddldirectory ../../../../database/schema --output ../../src/shared/generated/Schema.h

../../src/shared/generated/Encoder.cpp: ../../src/shared/generated/Encoder_cpp.template ../../../../../../engine/server/library/codegen/package_data.txt ../../../../../../engine/server/library/codegen/make_packages.pl
	cp ../../src/shared/generated/Encoder_cpp.template ../../src/shared/generated/Encoder.cpp
	chmod +w ../../src/shared/generated/Encoder.cpp
	perl ../../../../../../engine/server/library/codegen/make_packages.pl --datafile ../../../../../../engine/server/library/codegen/package_data.txt --encoder ../../src/shared/generated/Encoder.cpp

../../src/shared/generated/Decoder.cpp: ../../src/shared/generated/Decoder_cpp.template ../../../../../../engine/server/library/codegen/package_data.txt ../../../../../../engine/server/library/codegen/make_packages.pl
	cp ../../src/shared/generated/Decoder_cpp.template ../../src/shared/generated/Decoder.cpp
	chmod +w ../../src/shared/generated/Decoder.cpp
	perl ../../../../../../engine/server/library/codegen/make_packages.pl --datafile ../../../../../../engine/server/library/codegen/package_data.txt --decoder ../../src/shared/generated/Decoder.cpp

../../src/shared/generated/LoadObject.cpp: ../../src/shared/generated/LoadObject_cpp.template ../../../../../../engine/server/library/codegen/package_data.txt ../../../../../../engine/server/library/codegen/make_packages.pl
	cp ../../src/shared/generated/LoadObject_cpp.template ../../src/shared/generated/LoadObject.cpp
	chmod +w ../../src/shared/generated/LoadObject.cpp
	perl ../../../../../../engine/server/library/codegen/make_packages.pl --datafile ../../../../../../engine/server/library/codegen/package_data.txt --loadobject ../../src/shared/generated/LoadObject.cpp

../../src/shared/generated/ObjectQueries.cpp: ../../src/shared/generated/ObjectQueries_cpp.template ../../../../../../engine/server/library/codegen/make_queries.pl ../../../../database/packages/persister.plsqlh
	cp ../../src/shared/generated/ObjectQueries_cpp.template ../../src/shared/generated/ObjectQueries.cpp
	chmod +w ../../src/shared/generated/ObjectQueries.cpp
	perl ../../../../../../engine/server/library/codegen/make_queries.pl --plsql ../../../../database/packages/persister.plsqlh --source ../../src/shared/generated/ObjectQueries.cpp

../../src/shared/generated/ObjectQueries.h: ../../src/shared/generated/ObjectQueries_h.template ../../../../../../engine/server/library/codegen/make_queries.pl ../../../../database/packages/persister.plsqlh
	cp ../../src/shared/generated/ObjectQueries_h.template ../../src/shared/generated/ObjectQueries.h
	chmod +w ../../src/shared/generated/ObjectQueries.h
	perl ../../../../../../engine/server/library/codegen/make_queries.pl --plsql ../../../../database/packages/persister.plsqlh --header ../../src/shared/generated/ObjectQueries.h


#######################################################
#  include the common makefile
#######################################################

include ../../../../../build/shared_unix/application.mak
