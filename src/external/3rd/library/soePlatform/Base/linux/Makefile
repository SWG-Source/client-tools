########################################
#  Project specific macros

OUTPUT_DEBUG=libbase.so
OUTPUT_RELEASE=libbase.so

SOURCE_SHARED_DIR=..
SOURCE_PLATFORM_DIR=.
INCLUDE_DIRS=$(SOURCE_SHARED_DIR) $(SOURCE_PLATFORM_DIR)

LIBRARY_DIRS_DEBUG=
LIBRARY_DIRS_RELEASE=
LIBS_DEBUG=
LIBS_RELEASE=

DEBUG_DIR=../../compile/Base/linux/Debug
RELEASE_DIR=../../compile/Base/linux/Release


########################################
#  Platform specific macros

LIBS_PLATFORM=pthread

CFLAGS_DEFINES=-Wall -Wno-unknown-pragmas -D_REENTRANT -D_GNU_SOURCE
CFLAGS_PLATFORM_DEBUG=-g -D_DEBUG=1
CFLAGS_PLATFORM_RELEASE=-O2
LINK_FLAGS=-shared

CC=gcc
CPP=g++
LINK=ld


########################################
#  List Macros 

SRC_LIST_C=$(shell find $(SOURCE_SHARED_DIR) -name "*.c" -maxdepth 1) $(shell find $(SOURCE_PLATFORM_DIR) -name "*.c" -maxdepth 1)
SRC_LIST_CPP=$(shell find $(SOURCE_SHARED_DIR) -name "*.cpp" -maxdepth 1) $(shell find $(SOURCE_PLATFORM_DIR) -name "*.cpp" -maxdepth 1)
DEBUG_OBJ_LIST_C=$(addprefix $(DEBUG_DIR)/, $(notdir $(SRC_LIST_C:.c=.o)))
RELEASE_OBJ_LIST_C=$(addprefix $(RELEASE_DIR)/, $(notdir $(SRC_LIST_C:.c=.o)))
DEBUG_OBJ_LIST_CPP=$(addprefix $(DEBUG_DIR)/, $(notdir $(SRC_LIST_CPP:.cpp=.o)))
RELEASE_OBJ_LIST_CPP=$(addprefix $(RELEASE_DIR)/, $(notdir $(SRC_LIST_CPP:.cpp=.o)))


########################################
#  Flag Macros 

INCLUDE_FLAGS=$(addprefix -I,$(INCLUDE_DIRS))
CFLAGS= $(INCLUDE_FLAGS) $(CFLAGS_DEFINES)
CFLAGS_DEBUG= $(CFLAGS) $(CFLAGS_PLATFORM_DEBUG)
CFLAGS_RELEASE= $(CFLAGS) $(CFLAGS_PLATFORM_RELEASE)

LFLAGS_DEBUG=$(addprefix -L,$(LIBRARY_DIRS_DEBUG)) $(addprefix -l,$(LIBS_DEBUG)) $(addprefix -l,$(LIBS_PLATFORM))
LFLAGS_RELEASE=$(addprefix -L,$(LIBRARY_DIRS_RELEASE)) $(addprefix -l,$(LIBS_RELEASE)) $(addprefix -l,$(LIBS_PLATFORM))



########################################
#  Target Rules

all: debug release

debug: $(DEBUG_DIR) $(DEBUG_DIR)/$(OUTPUT_DEBUG) 
	@echo Successfully built $(DEBUG_DIR)/$(OUTPUT_DEBUG)

release: $(RELEASE_DIR) $(RELEASE_DIR)/$(OUTPUT_RELEASE)
	@echo Successfully built $(RELEASE_DIR)/$(OUTPUT_RELEASE)

$(DEBUG_OBJ_LIST_C): $(SRC_LIST)
	@echo compiling $(filter %/$(notdir $(basename $@)).c,$(SRC_LIST_C))
	@$(CC) $(CFLAGS_DEBUG) -o $@ -c $(filter %/$(notdir $(basename $@)).cpp,$(SRC_LIST_C))

$(DEBUG_OBJ_LIST_CPP): $(SRC_LIST)
	@echo compiling $(filter %/$(notdir $(basename $@)).cpp,$(SRC_LIST_CPP))
	@$(CPP) $(CFLAGS_DEBUG) -o $@ -c $(filter %/$(notdir $(basename $@)).cpp,$(SRC_LIST_CPP))

$(DEBUG_DIR) :
	@mkdir -p $@

$(DEBUG_DIR)/$(OUTPUT_DEBUG): $(DEBUG_OBJ_LIST_C) $(DEBUG_OBJ_LIST_CPP) $(DEBUG_DIR) 
	@echo linking $(DEBUG_DIR)/$(OUTPUT_DEBUG)
	@$(LINK) $(LINK_FLAGS) -o $@ $(DEBUG_OBJ_LIST_C) $(DEBUG_OBJ_LIST_CPP) $(LFLAGS_DEBUG)

$(RELEASE_OBJ_LIST_C): $(SRC_LIST)
	@echo compiling $(filter %/$(notdir $(basename $@)).c,$(SRC_LIST_C))
	@$(CC) $(CFLAGS_RELEASE) -o $@ -c $(filter %/$(notdir $(basename $@)).cpp,$(SRC_LIST_C))

$(RELEASE_OBJ_LIST_CPP): $(SRC_LIST)
	@echo compiling $(filter %/$(notdir $(basename $@)).cpp,$(SRC_LIST_CPP))
	@$(CPP) $(CFLAGS_RELEASE) -o $@ -c $(filter %/$(notdir $(basename $@)).cpp,$(SRC_LIST_CPP))

$(RELEASE_DIR) :
	@mkdir -p $@

$(RELEASE_DIR)/$(OUTPUT_RELEASE): $(RELEASE_OBJ_LIST_C) $(RELEASE_OBJ_LIST_CPP) $(RELEASE_DIR)
	@echo linking $(RELEASE_DIR)/$(OUTPUT_RELEASE)
	@$(LINK) $(LINK_FLAGS) -o $@ $(RELEASE_OBJ_LIST_C) $(RELEASE_OBJ_LIST_CPP) $(LFLAGS_RELEASE)

clean:
	rm -f $(DEBUG_DIR)/*.o
	rm -f $(RELEASE_DIR)/*.o
	rm -f $(DEBUG_DIR)/$(OUTPUT_DEBUG)
	rm -f $(RELEASE_DIR)/$(OUTPUT_RELEASE)


