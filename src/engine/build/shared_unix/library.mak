#This makefile assumes that defs.mak is already included by the project specific makefile

ifeq ($(VERBOSE_BUILD),)
V =@
else
V =
endif

ifeq ($(W_ERROR),)
W_ERROR=-Werror
endif

ifeq ($(STATIC_LIB),)
CUSTOM_CPP_FLAGS  += -fPIC
CUSTOM_LINK_FLAGS = -shared 

DYN_LINK          = -lc
DEBUG_OUTPUT      = lib$(LIBRARY).so.0.0
LINK_FLAGS	  = $(CUSTOM_LINK_FLAGS)
DEBUG_SYM_LINK	  = $(V)cd $(SHARED_DEBUG_LIB_PATH) && ln -sf ../../../../src/compile/linux/$(LIBRARY)/debug/$(DEBUG_OUTPUT) lib$(LIBRARY).so 

ifeq ($(BULLSEYE_BUILD),)
LINK	 	  = g++
else
LINK	 	  = /opt/app/bullseyecoverage/bin/g++
endif

else
CUSTOM_LINK_FLAGS =
DYN_LINK          =
CUSTOM_CPP_FLAGS  =
DEBUG_OUTPUT      = lib$(LIBRARY)_d.a
LINK_FLAGS	  = -r -static
DEBUG_SYM_LINK	  =
LINK	 	  = ld
endif


INCLUDE_DIRS_FINAL      =$(INCLUDE_DIRS) $(STL_INCLUDE_PATH) /usr/include /usr/include/libxml2

SHARED_SRC_LIST 	=$(shell find $$PWD/../../src/shared -name "*.cpp")
PLATFORM_SRC_LIST 	=$(shell [ -d $$PWD/../../src/$(PLATFORM) ] && find ../../src/$(PLATFORM) -name "*.cpp")
SRC_LIST 		=$(SHARED_SRC_LIST) $(PLATFORM_SRC_LIST) $(CUSTOM_SRC_LIST)
DEPENDENCY_LIST		= $(SRC_LIST:.cpp=.d)

DEBUG_OUTPUT_DIR	= $(BOOTPRINT_LIB_PATH)/$(LIBRARY)/debug

DEBUG_OBJ_LIST		= $(addprefix $(DEBUG_OUTPUT_DIR)/, $(notdir $(SRC_LIST:.cpp=.o)))
CPP_FLAGS_DEBUG		= -g $(addprefix -I,$(INCLUDE_DIRS_FINAL)) $(PLATFORM_DEFINES) $(CUSTOM_CPP_FLAGS) -D_REENTRANT -D_XOPEN_SOUCE=500 -D_GNU_SOURCE -DUSING_STL -D_DEBUG  -D__STL_NO_BAD_ALLOC -Wall -Wno-unknown-pragmas $(W_ERROR) -Wno-ctor-dtor-privacy -Wformat -Wreturn-type -Wimplicit -ftemplate-depth-25
#LINK_FLAGS		= -r $(CUSTOM_LINK_FLAGS) #for .so libs you need -shared
#DEBUG_OUTPUT		= lib$(LIBRARY)_d.a

RELEASE_OUTPUT_DIR	= $(BOOTPRINT_LIB_PATH)/$(LIBRARY)/release
RELEASE_OBJ_LIST	= $(addprefix $(RELEASE_OUTPUT_DIR)/, $(notdir $(SRC_LIST:.cpp=.o)))
CPP_FLAGS_RELEASE	= -g -O2 $(addprefix -I,$(INCLUDE_DIRS_FINAL)) -Wno-ctor-dtor-privacy $(PLATFORM_DEFINES) -D__STL_NO_BAD_ALLOC -D_REENTRANT -D_XOPEN_SOUCE=500 -D_GNU_SOURCE $(CUSTOM_CPP_FLAGS) -Wno-unknown-pragmas -DUSING_STL
RELEASE_OUTPUT		= lib$(LIBRARY)_r.a

LINT_SCRIPT             = $(BOOTPRINT_SRC_PATH)/../tools/swglint.sh
LINT_TARGETS            = $(addsuffix .lint,$(SRC_LIST))
.PHONY : release


%.cpp.lint : %.cpp
	@echo prerequisite: $(subst .lint,,$@)
	@echo output: $@
	$(LINT_SCRIPT) $(subst .lint,,$@)
	touch $@

all : debug release tags

publish:

lint: $(LINT_TARGETS)

$(LINT_TARGETS): $(SRC_LIST)
	$(LINT_SCRIPT) $(subst .lint,,$@)
	touch $@

debug: debug_custom_step
	+$(V)$(MAKE) $(SHARED_DEBUG_LIB_PATH)/$(DEBUG_OUTPUT)

$(SHARED_DEBUG_LIB_PATH):
	$(V)mkdir -p $(SHARED_DEBUG_LIB_PATH)

$(SHARED_DEBUG_LIB_PATH)/$(DEBUG_OUTPUT): $(SHARED_DEBUG_LIB_PATH) $(DEBUG_OUTPUT_DIR)/$(DEBUG_OUTPUT)

$(DEBUG_OUTPUT_DIR):
	$(V)mkdir -p $(DEBUG_OUTPUT_DIR)

$(DEBUG_OBJ_LIST):
	@echo $(notdir $(basename $@)).cpp
	$(V)$(CPP) $(CPP_FLAGS_DEBUG) -o $@ -c $(filter %/$(notdir $(basename $@)).cpp,$(SRC_LIST))

$(DEBUG_OUTPUT_DIR)/$(DEBUG_OUTPUT): $(DEBUG_OUTPUT_DIR) $(DEBUG_OBJ_LIST)
	@echo Creating debug library $@
	$(V)$(LINK) $(LINK_FLAGS) -o $(DEBUG_OUTPUT_DIR)/$(DEBUG_OUTPUT) $(DYN_LINK) $(DEBUG_OBJ_LIST)
	$(DEBUG_SYM_LINK)

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),depend)
include .depend
endif
endif

.PHONY : $(DEPENDENCY_LIST)

#depend: .depend $(DEPENDENCY_LIST)
#	$(V)perl $(BOOTPRINT_SRC_PATH)/../tools/programmer/linux/dependcat.pl $(DEPENDENCY_LIST) > .depend
#	$(V)rm $(DEPENDENCY_LIST)

depend: .depend $(DEPENDENCY_LIST)
	$(V)perl $(BOOTPRINT_SRC_PATH)/../tools/dependcat.pl $(DEPENDENCY_LIST) > .depend
	$(V)rm $(DEPENDENCY_LIST)

.depend:
	touch .depend
	+make depend_custom_step
	+make depend

$(DEPENDENCY_LIST):
	@echo Generating dependencies for $(notdir $(@:.d=.cpp))
	@echo -n $(DEBUG_OUTPUT_DIR)/ >> $@
	$(V)$(CPP) $(PLATFORM_DEFINES) $(CPP_FLAGS_RELEASE) -MM $(addprefix -I,$(INCLUDE_DIRS_FINAL)) $(@:.d=.cpp)>> $@

clean: custom_clean_step
	touch .depend
	rm -Rf $(DEBUG_OUTPUT_DIR)
	rm -f .depend

#Create emacs Tags for source files for easy browsing
tags : $(SRC_LIST)
	@mkdir -p Tags
	@etags $(SRC_LIST)
	@mv -f TAGS Tags
