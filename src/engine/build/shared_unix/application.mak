ifeq ($(VERBOSE_BUILD),)
V =@
else
V =
endif

DEBUG_OBJ_DIR 		= ../../../../../../compile/linux/$(OUTPUT)/debug
RELEASE_OBJ_DIR 	= ../../../../../../compile/linux/$(OUTPUT)/release

TOOLS_DIR 		= ../../../../../../../tools

FINAL_OUTPUT_DEBUG      = $(OUTPUT)_d
FINAL_OUTPUT_RELEASE    = $(OUTPUT)_r

DEPENDENCY_LIST 	= $(SRC_LIST:.cpp=.d)

RELEASE_OBJ_LIST	= $(addprefix $(RELEASE_OBJ_DIR)/, $(notdir $(SRC_LIST:.cpp=.o)))
DEBUG_OBJ_LIST		= $(addprefix $(DEBUG_OBJ_DIR)/, $(notdir $(SRC_LIST:.cpp=.o)))

INCLUDE_DIRS           += /usr/include /usr/include/libxml2

CPP_FLAGS_RELEASE	= -g -O2 $(addprefix -I,$(INCLUDE_DIRS)) $(RELEASE_PLATFORM_CPP_FLAGS) $(PLATFORM_DEFINES) -Wno-ctor-dtor-privacy -D__STL_NO_BAD_ALLOC
CPP_FLAGS_DEBUG		= -g $(addprefix -I,$(INCLUDE_DIRS)) $(DEBUG_PLATFORM_CPP_FLAGS) $(PLATFORM_DEFINES) -Wall -Wno-unknown-pragmas -Werror -Wno-ctor-dtor-privacy -D_DEBUG -D__STL_NO_BAD_ALLOC -Wformat -Wreturn-type -Wimplicit

#DEBUG_LINK_FLAGS        += -lncurses -rdynamic -Xlinker -rpath -Xlinker ./
DEBUG_LINK_FLAGS        +=  -lxml2 -lz -lncurses -Wl,--rpath -Wl,$(SHARED_DEBUG_LIB_PATH)

RELEASE_LINK_FLAGS      += -rdynamic -Xlinker -rpath -Xlinker ./

BASE_OUTPUT_DIR		= ../../../../../../../dev/linux
DEBUG_FINAL_OUTPUT_DIR  = $(BASE_OUTPUT_DIR)/debug

DEBUG_SYM_LINK	  	= $(V)cd $(BASE_OUTPUT_DIR) && ln -sf debug/$(FINAL_OUTPUT_DEBUG) $(OUTPUT)


LINT_SCRIPT             = $(BOOTPRINT_SRC_PATH)/../tools/swglint.sh
LINT_TARGETS            = $(addsuffix .lint,$(SRC_LIST))

#Following separates the shared libraries into names and paths
#so that we can link them with just their names, not the entire path:

DEBUG_SHARED_LIB_LINK_FLAGS  = $(addprefix -l, $(notdir $(subst lib,,$(DEBUG_SHARED_LIBRARIES:.so=))))
DEBUG_SHARED_LIB_LINK_DIRS 	= -L$(SHARED_DEBUG_LIB_PATH)
DEBUG_DEPS 			= $(DEBUG_LINK_LIBRARIES:.a=.a_dep)
SHARED_DEBUG_DEPS 		= $(DEBUG_SHARED_LIBRARIES:.so=.so_dep)


all: debug

lint: $(LINT_TARGETS)


%.cpp.lint : %.cpp
	@echo prerequisite: $(subst .lint,,$@)
	@echo output: $@
	$(LINT_SCRIPT) $(subst .lint,,$@)
	touch $@

#######################################################
#  Debug
#######################################################

debug: .depend $(DEBUG_DEPS) $(SHARED_DEBUG_DEPS) debug_custom_step
	+$(V)$(MAKE) $(DEBUG_FINAL_OUTPUT_DIR)/$(FINAL_OUTPUT_DEBUG)

$(DEBUG_FINAL_OUTPUT_DIR)/$(FINAL_OUTPUT_DEBUG): $(DEBUG_FINAL_OUTPUT_DIR) $(DEBUG_OBJ_DIR)/$(OUTPUT)
	@echo Creating debug: $(OUTPUT) $@
	$(V)cp -f $(DEBUG_OBJ_DIR)/$(OUTPUT) $(DEBUG_FINAL_OUTPUT_DIR)/$(FINAL_OUTPUT_DEBUG)
	$(DEBUG_SYM_LINK)

$(DEBUG_FINAL_OUTPUT_DIR):
	$(V)mkdir -p $(DEBUG_FINAL_OUTPUT_DIR)

$(DEBUG_OBJ_DIR)/$(OUTPUT): $(DEBUG_OBJ_DIR) $(DEBUG_OBJ_LIST) $(DEBUG_LINK_LIBRARIES)
	@echo Creating debug: $(OUTPUT) $@
	$(V)$(CPP) $(CPP_FLAGS_DEBUG) $(DEBUG_LINK_FLAGS) -o $(DEBUG_OBJ_DIR)/$(OUTPUT) $(DEBUG_SHARED_LIB_LINK_DIRS) $(DEBUG_SHARED_LIB_LINK_FLAGS) $(DEBUG_OBJ_LIST) $(DEBUG_LINK_LIBRARIES)
	$(DEBUG_SYM_LINK)

$(DEBUG_OBJ_DIR):
	$(V)mkdir -p $(DEBUG_OBJ_DIR)

$(DEBUG_OBJ_LIST):
	@echo $(notdir $(basename $@)).cpp
	$(V)$(CPP) $(CPP_FLAGS_DEBUG) -o $@ -c $(filter %/$(notdir $(basename $@)).cpp,$(SRC_LIST))


#######################################################
#  .depend
#######################################################

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),depend)
include .depend
endif
endif

#depend: .depend $(DEPENDENCY_LIST)
#	$(V)perl $(BOOTPRINT_SRC_PATH)/../tools/programmer/linux/dependcat.pl $(DEPENDENCY_LIST) > .depend
#	$(V)rm $(DEPENDENCY_LIST)

depend: .depend $(DEPENDENCY_LIST)
	$(V)perl $(BOOTPRINT_SRC_PATH)/../tools/dependcat.pl $(DEPENDENCY_LIST) > .depend
	$(V)rm $(DEPENDENCY_LIST)

.depend:
	touch .depend
	+$(MAKE) depend_custom_step
	+$(MAKE) depend

.PHONY : $(DEPENDENCY_LIST)

$(DEPENDENCY_LIST):
	@echo Generating dependencies for $(notdir $(@:.d=.cpp))
	@echo -n $(DEBUG_OBJ_DIR)/ >> $@
	$(V)$(CPP) $(PLATFORM_DEFINES) $(CPP_FLAGS_RELEASE) -MM $(addprefix -I,$(INCLUDE_DIRS)) $(@:.d=.cpp) >> $@

#######################################################
#  clean
#######################################################

clean: cleandepend cleandebug

cleandepend:
	rm -f .depend

cleandebug:
	rm -Rf $(DEBUG_OBJ_DIR)
	rm -f $(DEBUG_FINAL_OUTPUT_DIR)/$(FINAL_OUTPUT_DEBUG)


