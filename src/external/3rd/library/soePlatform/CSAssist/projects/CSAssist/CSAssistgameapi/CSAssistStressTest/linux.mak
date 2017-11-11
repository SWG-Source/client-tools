# # # # # # # # # # # # # # # # # # # # #

OUTPUT_FILE=CSAssistclient
OUTPUT_DIR=.
OBJECT_DIR=./obj

INCLUDE_DIRS=../.. ../

SOURCE_DIRS=. 
SOURCE_FILES=

LIBRARY_DIRS=../debug ../../lib/debug
LIBRARY_FILES=CSAssistgameapi Base UdpLibrary pthread

CFLAGS_USER=
LFLAGS_USER=

# # # # # # # # # # # # # # # # # # # # #

INCLUDE_FLAGS=$(addprefix -I,$(INCLUDE_DIRS))
CFLAGS=$(INCLUDE_FLAGS) $(CFLAGS_USER) -D_REENTRANT -Wall -Wno-unknown-pragmas
CFLAGS_DEBUG=$(CFLAGS) -D_DEBUG -g
CFLAGS_RELEASE=$(CFLAGS) -DNDEBUG -O2

LIBRARY_FLAGS=$(addprefix -L,$(LIBRARY_DIRS)) $(addprefix -l,$(LIBRARY_FILES))
LFLAGS=$(LIBRARY_FLAGS) $(LFLAGS_USER)
LFLAGS_DEBUG=$(LFLAGS)
LFLAGS_RELEASE=$(LFLAGS)

CPP=g++
LINK=g++

# # # # # # # # # # # # # # # # # # # # #

DEBUG_OBJ_DIR=$(OBJECT_DIR)/debug
RELEASE_OBJ_DIR=$(OBJECT_DIR)/release

DEBUG_DIR=$(OUTPUT_DIR)/debug
RELEASE_DIR=$(OUTPUT_DIR)/release

SOURCE_LIST=$(SOURCE_FILES) $(foreach x, $(SOURCE_DIRS), $(shell find $(x) -name "*.cpp" -maxdepth 1))
DEBUG_OBJ_LIST=$(addprefix $(DEBUG_OBJ_DIR)/, $(notdir $(SOURCE_LIST:.cpp=.o)))
RELEASE_OBJ_LIST=$(addprefix $(RELEASE_OBJ_DIR)/, $(notdir $(SOURCE_LIST:.cpp=.o)))

# # # # # # # # # # # # # # # # # # # # #

release: release_dir $(RELEASE_DIR)/$(OUTPUT_FILE)
	@echo Successfully built $(RELEASE_DIR)/$(OUTPUT_FILE)
	@echo

debug: debug_dir $(DEBUG_DIR)/$(OUTPUT_FILE)
	@echo Successfully built $(DEBUG_DIR)/$(OUTPUT_FILE) 
	@echo

all: clean debug release

debug_dir:
	@mkdir -p $(DEBUG_OBJ_DIR)
	@mkdir -p $(DEBUG_DIR)

$(DEBUG_OBJ_DIR)/%.o : $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))
	@echo compiling $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))
	$(CPP) $(CFLAGS_DEBUG) -o $@ -c $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))

$(DEBUG_DIR)/$(OUTPUT_FILE): $(DEBUG_OBJ_LIST)
	@echo linking $(DEBUG_DIR)/$(OUTPUT_FILE)
	$(LINK) $(LFLAGS_DEBUG) -o $@ $(DEBUG_OBJ_LIST)

release_dir:
	@mkdir -p $(RELEASE_OBJ_DIR)
	@mkdir -p $(RELEASE_DIR)

$(RELEASE_OBJ_DIR)/%.o : $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))
	@echo compiling $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))
	@$(CPP) $(CFLAGS_RELEASE) -o $@ -c $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))

$(RELEASE_DIR)/$(OUTPUT_FILE): $(RELEASE_OBJ_LIST)
	@echo linking $(RELEASE_DIR)/$(OUTPUT_FILE)
	@$(LINK) $(LFLAGS_RELEASE) -o $@ $(RELEASE_OBJ_LIST)

clean:
	@echo removing "$(DEBUG_OBJ_DIR)/*.o"
	@rm -f $(DEBUG_OBJ_DIR)/*.o
	@echo removing "$(RELEASE_OBJ_DIR)/*.o"
	@rm -f $(RELEASE_OBJ_DIR)/*.o
	@echo removing $(DEBUG_DIR)/$(OUTPUT_FILE)
	@rm -f $(DEBUG_DIR)/$(OUTPUT_FILE)
	@echo removing $(RELEASE_DIR)/$(OUTPUT_FILE)
	@rm -f $(RELEASE_DIR)/$(OUTPUT_FILE)
	@echo


