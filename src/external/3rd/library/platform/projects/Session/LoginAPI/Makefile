# environment variables:
#	$INCLUDE_PATH is a colon-delimited list of include directories

EMPTY_CHAR=
SPACE_CHAR=$(empty) $(empty)
COLON_CHAR=:

# # # # # # # # # # # # # # # # # # # # #

OUTPUT_FILE=libLoginAPI.a
OUTPUT_DIR=../../../lib
OBJECT_DIR=.

STL_HOME=../../../../stlport453

INCLUDE_DIRS=../.. ../../../utils ../../../../udplibrary $(STL_HOME)/stlport

SOURCE_DIRS=.
SOURCE_FILES=

LIBRARY_DIRS=
LIBRARY_FILES=

CFLAGS_USER=
LFLAGS_USER=

# # # # # # # # # # # # # # # # # # # # #

INCLUDE_DIR_LIST=$(subst $(COLON_CHAR),$(SPACE_CHAR),$(INCLUDE_PATH)) $(INCLUDE_DIRS) 
INCLUDE_FLAGS=$(addprefix -I,$(INCLUDE_DIR_LIST))
CFLAGS=$(INCLUDE_FLAGS) $(CFLAGS_USER) -D_GNU_SOURCE -Wall -Wno-unknown-pragmas
CFLAGS_DEBUG=$(CFLAGS) -D_DEBUG -g
CFLAGS_RELEASE=$(CFLAGS) -DNDEBUG -O2

LIBRARY_FLAGS=$(addprefix -l,$(LIBRARIES))
LFLAGS=$(LIBRARY_FLAGS) $(LFLAGS_USER) -r
LFLAGS_DEBUG=$(LFLAGS)
LFLAGS_RELEASE=$(LFLAGS)

CPP=g++
LINK=ld

# # # # # # # # # # # # # # # # # # # # #

DEBUG_OBJ_DIR=$(OBJECT_DIR)/debug
RELEASE_OBJ_DIR=$(OBJECT_DIR)/release

DEBUG_DIR=$(OUTPUT_DIR)/debug
RELEASE_DIR=$(OUTPUT_DIR)/release

FIND=$(shell ls $(x)/*.cpp)
SOURCE_LIST = $(SOURCE_FILES) $(foreach x, $(SOURCE_DIRS), $(FIND))

DEBUG_OBJ_LIST=$(addprefix $(DEBUG_OBJ_DIR)/, $(notdir $(SOURCE_LIST:.cpp=.o)))
RELEASE_OBJ_LIST=$(addprefix $(RELEASE_OBJ_DIR)/, $(notdir $(SOURCE_LIST:.cpp=.o)))

DEPENDENCY_LIST 	= $(SOURCE_LIST:.cpp=.d)

# # # # # # # # # # # # # # # # # # # # #

release: echoRelease releaseDirs $(RELEASE_DIR)/$(OUTPUT_FILE)
	@echo Successfully built $(RELEASE_DIR)/$(OUTPUT_FILE)
	@echo

debug: debugDirs $(DEBUG_DIR)/$(OUTPUT_FILE)
	@echo Successfully build $(DEBUG_DIR)/$(OUTPUT_FILE)
	@echo

all: clean debug release

debugDirs :
	@mkdir -p $(DEBUG_OBJ_DIR)
	@mkdir -p $(DEBUG_DIR)

$(DEBUG_OBJ_DIR)/%.o : $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))
#	@echo compiling $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))
	$(CPP) $(CFLAGS_DEBUG) -o $@ -c $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))

$(DEBUG_DIR)/$(OUTPUT_FILE): $(DEBUG_OBJ_LIST)
#	@echo linking $(DEBUG_DIR)/$(OUTPUT_FILE)
	$(LINK) $(LFLAGS_DEBUG) -o $@ $(DEBUG_OBJ_LIST)

echoRelease :
	@echo Building $(OUTPUT_FILE)
	@echo compiler: $(CPP) $(CFLAGS_RELEASE)
	@echo linker: $(LINK) $(LFLAGS_RELEASE)
	@echo

releaseDirs :
	@mkdir -p $(RELEASE_OBJ_DIR)
	@mkdir -p $(RELEASE_DIR)

$(RELEASE_OBJ_DIR)/%.o : $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))
	@echo compiling $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))
	@$(CPP) $(CFLAGS_RELEASE) -o $@ -c $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))

$(RELEASE_DIR)/$(OUTPUT_FILE): $(RELEASE_OBJ_LIST)
	@echo linking $(RELEASE_DIR)/$(OUTPUT_FILE)
	@$(LINK) $(LFLAGS_RELEASE) -o $@ $(RELEASE_OBJ_LIST)


# # # # # # # # # # # # # # # # # # # # #

include .depend

.PHONY : $(DEPENDENCY_LIST)

depend: .depend $(DEPENDENCY_LIST)

.depend:
	touch .depend
	make depend

$(DEPENDENCY_LIST):
	@echo Generating dependencies for $(notdir $(@:.d=.cpp))
	@echo -n $(DEBUG_OBJ_DIR)/ >> .depend
	@$(CPP) -MM $(addprefix -I,$(INCLUDE_DIRS)) $(@:.d=.cpp)>> .depend
	@echo -n $(RELEASE_OBJ_DIR)/ >> .depend
	@$(CPP) -MM $(addprefix -I,$(INCLUDE_DIRS)) $(@:.d=.cpp)>> .depend

# # # # # # # # # # # # # # # # # # # # #

clean:
	@echo removing "$(DEBUG_OBJ_DIR)/*.o"
	@rm -f $(DEBUG_OBJ_DIR)/*.o
	@echo removing "$(RELEASE_OBJ_DIR)/*.o"
	@rm -f $(RELEASE_OBJ_DIR)/*.o
	@echo removing $(DEBUG_DIR)/$(OUTPUT_FILE)
	@rm -f $(DEBUG_DIR)/$(OUTPUT_FILE)
	@echo removing $(RELEASE_DIR)/$(OUTPUT_FILE)
	@rm -f $(RELEASE_DIR)/$(OUTPUT_FILE)
	@rm -f .depend
	@echo


