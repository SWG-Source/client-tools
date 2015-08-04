SRC_LIST=./UdpLibrary.cpp
LIB_DEBUG_DIR=output/debug
LIB_RELEASE_DIR=output/release
DEBUG_OBJ_LIST=$(addprefix $(LIB_DEBUG_DIR)/, $(notdir $(SRC_LIST:.cpp=.o)))
RELEASE_OBJ_LIST=$(addprefix $(LIB_RELEASE_DIR)/, $(notdir $(SRC_LIST:.cpp=.o)))
DEBUG_OBJ_LIST_C=$(addprefix $(LIB_DEBUG_DIR)/, $(notdir $(SRC_LIST_SHARED_C:.c=.o)))
RELEASE_OBJ_LIST_C=$(addprefix $(LIB_RELEASE_DIR)/, $(notdir $(SRC_LIST_SHARED_C:.c=.o)))
LIB_OUTPUT=libudplibrary.a
INCLUDE_DIRS=./
INCLUDE_FLAGS=$(addprefix -I,$(INCLUDE_DIRS))
CPP=g++
CC=gcc
CFLAGS= $(INCLUDE_FLAGS) -DREENTRANT
CFLAGS_DEBUG= $(CFLAGS) -g -D_DEBUG=1 -Wall -Werror -Wno-unknown-pragmas -DREENTRANT
CFLAGS_RELEASE= $(CFLAGS) -g -O2
LINK=ld
LINK_FLAGS=-r
DEPENDENCY_LIST 	= $(SRC_LIST:.cpp=.d)

all:
	make debug
	make release

debug: $(LIB_DEBUG_DIR) $(LIB_DEBUG_DIR)/$(LIB_OUTPUT)
	@echo Successfully built $(LIB_DEBUG_DIR)/$(LIB_OUTPUT)

release: $(LIB_RELEASE_DIR) $(LIB_RELEASE_DIR)/$(LIB_OUTPUT)
	@echo Successfully built $(LIB_RELEASE_DIR)/$(LIB_OUTPUT)

#Make debug build


$(DEBUG_OBJ_LIST): $(SRC_LIST)
	@echo compiling $(filter %/$(notdir $(basename $@)).cpp,$(SRC_LIST))
	@$(CPP) $(CFLAGS_DEBUG) -o $@ -c $(filter %/$(notdir $(basename $@)).cpp,$(SRC_LIST))

$(DEBUG_OBJ_LIST_C): $(SRC_LIST_SHARED_C)
	@echo compiling $(filter %/$(notdir $(basename $@)).c,$(SRC_LIST_SHARED_C))
	$(CC) $(CFLAGS_DEBUG) -o $@ -c $(filter %/$(notdir $(basename $@)).c,$(SRC_LIST_SHARED_C))

$(LIB_DEBUG_DIR) :
	mkdir -p $@

$(LIB_DEBUG_DIR)/$(LIB_OUTPUT): $(DEBUG_OBJ_LIST) $(DEBUG_OBJ_LIST_C) $(LIB_DEBUG_DIR)
	@echo linking $(LIB_DEBUG_DIR)/$(LIB_OUTPUT)
	$(LINK) $(LINK_FLAGS) -o $@ $(DEBUG_OBJ_LIST) $(DEBUG_OBJ_LIST_C)

#Make release build
$(RELEASE_OBJ_LIST): $(SRC_LIST)
	@echo compiling $(filter %/$(notdir $(basename $@)).cpp,$(SRC_LIST))
	@$(CPP) $(CFLAGS_RELEASE) -o $@ -c $(filter %/$(notdir $(basename $@)).cpp,$(SRC_LIST))

$(RELEASE_OBJ_LIST_C): $(SRC_LIST_SHARED_C)
	@echo compiling $(filter %/$(notdir $(basename $@)).c,$(SRC_LIST_SHARED_C))
	@$(CC) $(CFLAGS_RELEASE) -o $@ -c $(filter %/$(notdir $(basename $@)).c,$(SRC_LIST_SHARED_C))

$(LIB_RELEASE_DIR) :
	@mkdir -p $@

$(LIB_RELEASE_DIR)/$(LIB_OUTPUT): $(RELEASE_OBJ_LIST) $(RELEASE_OBJ_LIST_C) $(LIB_RELEASE_DIR)
	@echo linking $(LIB_RELEASE_DIR)/$(LIB_OUTPUT)
	@$(LINK) $(LINK_FLAGS) -o $@ $(RELEASE_OBJ_LIST) $(RELEASE_OBJ_LIST_C)

include .depend

.PHONY : $(DEPENDENCY_LIST)

depend: .depend $(DEPENDENCY_LIST)

.depend:
	touch .depend
	make depend

$(DEPENDENCY_LIST):
	@echo Generating dependencies for $(notdir $(@:.d=.cpp))
	@echo -n $(LIB_DEBUG_DIR)/ >> .depend
	@$(CPP) -MM $(addprefix -I,$(INCLUDE_DIRS)) $(@:.d=.cpp)>> .depend
	@echo -n $(LIB_RELEASE_DIR)/ >> .depend
	@$(CPP) -MM $(addprefix -I,$(INCLUDE_DIRS)) $(@:.d=.cpp) >> .depend

clean:
	rm -f $(LIB_DEBUG_DIR)/*
	rm -f $(LIB_RELEASE_DIR)/*
	rm -f .depend


