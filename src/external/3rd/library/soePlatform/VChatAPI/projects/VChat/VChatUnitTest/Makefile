# # # # # # # # # # # # # # # # # # # # #
# environment variables:
#	$INCLUDE_PATH is a colon-delimited list of include directories
#       $ORACLE_HOME is the path to the Oracle installation

EMPTY_CHAR=
SPACE_CHAR=$(empty) $(empty)
COLON_CHAR=:
EXCLUDE_CHAR_TRAITS=EXCLUDE
ORACLE_HOME=/opt/app/oracle/product/9.2.0

# # # # # # # # # # # # # # # # # # # # #

OUTPUT_FILE=VChatUnitTest
OUTPUT_DIR=./bin
OBJECT_DIR=./obj

UTILS_BASE=../../../utils2.0
UTILS_DIR=$(UTILS_BASE)/utils
ORACLE_INCLUDE_DIRS=$(ORACLE_HOME)/rdbms/demo $(ORACLE_HOME)/rdbms/public $(ORACLE_HOME)/network/public
XML_DIR=$(UTILS_DIR)/XMLLibrary
INCLUDE_DIRS=$(ORACLE_INCLUDE_DIRS) $(UTILS_DIR) ../VChatAPI $(XML_DIR) ../ ../../../../../stlport453/stlport

SOURCE_DIRS=. 

LIBRARY_DIRS=$(UTILS_DIR)/SoeMon3AgentAPI/monapi/lib $(UTILS_DIR)/SSLLibrary/openssl/lib/Debug 
LIBRARY_DIRS_DEBUG=$(ORACLE_HOME)/lib $(UTILS_BASE)/lib/debug ../lib/debug $(LIBRARY_DIRS) 
LIBRARY_DIRS_RELEASE=$(ORACLE_HOME)/lib $(UTILS_BASE)/lib/release ../lib/release $(LIBRARY_DIRS)
LIBRARY_FILES=Base_MT Resources_MT TcpLibrary_MT UdpLibrary_MT OracleDB_MT clntsh pthread ssl SSLSocket_MT SoeMonApi3 xml2 VChatAPI_MT AES_MT
LIBSRC_DIRS=$(addprefix $(UTILS_DIR)/, Base OracleDB SSLLibrary TcpLibrary UdpLibrary AES Resources) ../VChatAPI

CFLAGS_USER=-D_REENTRANT
LFLAGS_USER=

# # # # # # # # # # # # # # # # # # # # #

INCLUDE_DIR_LIST=$(subst $(COLON_CHAR),$(SPACE_CHAR),$(INCLUDE_PATH)) $(INCLUDE_DIRS) 
INCLUDE_FLAGS=$(addprefix -I,$(INCLUDE_DIR_LIST))
CFLAGS=$(INCLUDE_FLAGS) $(CFLAGS_USER) -D_GNU_SOURCE -Wall -Wno-unknown-pragmas -DAPI_NAMESPACE="VChatSystem"
CFLAGS_DEBUG=$(CFLAGS) -D_DEBUG -g
CFLAGS_RELEASE=$(CFLAGS) -DNDEBUG -O2

LIBRARY_FLAGS_DEBUG=$(addprefix -l,$(LIBRARY_FILES)) $(addprefix -L,$(LIBRARY_DIRS_DEBUG))
LIBRARY_FLAGS_RELEASE=$(addprefix -l,$(LIBRARY_FILES)) $(addprefix -L,$(LIBRARY_DIRS_RELEASE))
LFLAGS_DEBUG=$(LIBRARY_FLAGS_DEBUG) $(LFLAGS_USER)
LFLAGS_RELEASE=$(LIBRARY_FLAGS_RELEASE) $(LFLAGS_USER)

CPP=g++
LINK=g++

# # # # # # # # # # # # # # # # # # # # #

DEBUG_OBJ_DIR=$(OBJECT_DIR)/debug
RELEASE_OBJ_DIR=$(OBJECT_DIR)/release

DEBUG_DIR=$(OUTPUT_DIR)/debug
RELEASE_DIR=$(OUTPUT_DIR)/release

#FIND=$(shell ls $(x)/*.cpp)
FIND=$(shell ls $(x)/*.cpp | if [ "$(EXCLUDED_SOURCE_FILES)" != "" ]; then egrep -v "$(EXCLUDED_SOURCE_FILES)"; else egrep "*"; fi)
SOURCE_LIST = $(SOURCE_FILES) $(foreach x, $(SOURCE_DIRS), $(FIND))

DEBUG_OBJ_LIST=$(addprefix $(DEBUG_OBJ_DIR)/, $(notdir $(SOURCE_LIST:.cpp=.o)))
RELEASE_OBJ_LIST=$(addprefix $(RELEASE_OBJ_DIR)/, $(notdir $(SOURCE_LIST:.cpp=.o)))

MK_DEBUG_DIRS=$(shell if [ ! -d $(DEBUG_OBJ_DIR) ]; then mkdir -p $(DEBUG_OBJ_DIR) $(DEBUG_DIR); fi)
MK_RELEASE_DIRS=$(shell if [ ! -d $(RELEASE_OBJ_DIR) ]; then mkdir -p $(RELEASE_OBJ_DIR) $(RELEASE_DIR); fi)
# # # # # # # # # # # # # # # # # # # # #

release: echoRelease releaseDirs libsrelease $(RELEASE_DIR)/$(OUTPUT_FILE)
	@echo Successfully built $(RELEASE_DIR)/$(OUTPUT_FILE)
	@echo

debug: debugDirs libsdebug $(DEBUG_DIR)/$(OUTPUT_FILE)
	@echo Successfully built $(DEBUG_DIR)/$(OUTPUT_FILE)
	@echo

all: debug release

include dist.mk
include version.mk
-include distcc.mk
# # # # # # # # # # # # # # # # # # # # #

debugDirs :
	@mkdir -p $(DEBUG_OBJ_DIR)
	@mkdir -p $(DEBUG_DIR)

$(DEBUG_OBJ_DIR)/%.o : $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))
	$(CPP) $(CFLAGS_DEBUG) -o $@ -c $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))

$(DEBUG_DIR)/$(OUTPUT_FILE): $(DEBUG_OBJ_LIST)
	$(LINK) $(LFLAGS_DEBUG) -o $@ $(DEBUG_OBJ_LIST)

$(DEBUG_OBJ_DIR)/MTDatabaseResourceManager.d : ../../../utils/GenericServer/ReusableResourceManagers/MTDatabaseResourceManager.cpp
	$(SHELL) -ec '$(CPP) -MT "$(DEBUG_OBJ_DIR)/$*.o $(DEBUG_OBJ_DIR)/*.d" -MM $(CFLAGS_DEBUG) $<;' > $@

# # # # # # # # # # # # # # # # # # # # #

echoRelease :
	@echo Building $(OUTPUT_FILE)
	@echo compiler: $(CPP) $(CFLAGS_RELEASE)
	@echo linker: $(LINK) $(LFLAGS_RELEASE)
	@echo


# # # # # # # # # # # # # # # # # # # # #

releaseDirs :
	@mkdir -p $(RELEASE_OBJ_DIR)
	@mkdir -p $(RELEASE_DIR)

$(RELEASE_OBJ_DIR)/%.o : $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))
	@echo compiling $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))
	@$(CPP) $(CFLAGS_RELEASE) -o $@ -c $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))

$(RELEASE_DIR)/$(OUTPUT_FILE): $(RELEASE_OBJ_LIST)
	@echo linking $(RELEASE_DIR)/$(OUTPUT_FILE)
	@$(LINK) $(LFLAGS_RELEASE) -o $@ $(RELEASE_OBJ_LIST)

$(RELEASE_OBJ_DIR)/MTDatabaseResourceManager.d : ../../../utils/GenericServer/ReusableResourceManagers/MTDatabaseResourceManager.cpp
	$(SHELL) -ec '$(CPP) -MT "$(RELEASE_OBJ_DIR)/$*.o $(RELEASE_OBJ_DIR)/*.d" -MM $(CFLAGS_RELEASE) $<;' > $@



# # # # # # # # # # # # # # # # # # # # #

clean:
#	@echo removing "$(OBJECT_DIR)/*.o"
	rm -rf $(DEBUG_OBJ_DIR)/*.o
	rm -rf $(RELEASE_OBJ_DIR)/*.o
	@echo removing $(DEBUG_DIR)/$(OUTPUT_FILE)
	@rm -f $(DEBUG_DIR)/$(OUTPUT_FILE)
	@echo removing $(RELEASE_DIR)/$(OUTPUT_FILE)
	@rm -f $(RELEASE_DIR)/$(OUTPUT_FILE)
	@echo

depclean:
	@echo removing dependancy files "$(OBJECT_DIR)/*/*.d"
	@rm -rf $(OBJECT_DIR)/*/*.d
# # # # # # # # # # # # # # # # # # # # #
# rule to generate the dependency files
# there are 2 sets of dependency files
#debug object directories and release directories
# # # # # # # # # # # # # # # # # # # # #
#$(DEBUG_OBJ_DIR)/%.d: %.cpp
#	$(SHELL) -ec '$(CPP) -MT "$(DEBUG_OBJ_DIR)/$*.o $(DEBUG_OBJ_DIR)/$*.d" -MM $(CFLAGS_DEBUG) $<;' > $@
#$(RELEASE_OBJ_DIR)/%.d: %.cpp
#	$(SHELL) -ec '$(CPP) -MT "$(RELEASE_OBJ_DIR)/$*.o $(RELEASE_OBJ_DIR)/$*.d" -MM $(CFLAGS_DEBUG) $<;' > $@
$(DEBUG_OBJ_DIR)/%.d:	$(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))
	@if [ ! -d "$(DEBUG_OBJ_DIR)" ]; then mkdir -p $(DEBUG_OBJ_DIR); fi
	$(SHELL) -ec '$(CPP) -MT "$(DEBUG_OBJ_DIR)/$*.o $(DEBUG_OBJ_DIR)/$*.d" -MM $(CFLAGS_DEBUG) $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST));' > $@ 

$(RELEASE_OBJ_DIR)/%.d:	$(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST))
	@if [ ! -d "$(RELEASE_OBJ_DIR)" ]; then mkdir -p $(RELEASE_OBJ_DIR); fi
	$(SHELL) -ec '$(CPP) -MT "$(RELEASE_OBJ_DIR)/$*.o $(RELEASE_OBJ_DIR)/$*.d" -MM $(CFLAGS_DEBUG) $(filter %/$(notdir $(basename $@)).cpp,$(SOURCE_LIST));' > $@ 



# # # # # # # # # # # # # # # # # # # # #
-include $(DEBUG_OBJ_LIST:.o=.d)
-include $(RELEASE_OBJ_LIST:.o=.d)

