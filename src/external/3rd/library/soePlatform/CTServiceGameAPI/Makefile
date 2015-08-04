TARGET = libCTServiceGameAPI.a

OBJS = \
	Base/Archive.o \
	Base/linux/Platform.o \
	TcpLibrary/Clock.o \
	TcpLibrary/IPAddress.o \
	TcpLibrary/TcpBlockAllocator.o \
	TcpLibrary/TcpConnection.o \
	TcpLibrary/TcpManager.o \
	Unicode/FirstUnicode.o \
	Unicode/Unicode.o \
	Unicode/UnicodeBlocks.o \
	Unicode/UnicodeCharacterData.o \
	Unicode/UnicodeCharacterDataMap.o \
	Unicode/UnicodeUtils.o \
	CTCommon/CTServiceCharacter.o \
	CTCommon/CTServiceServer.o \
	CTGenericAPI/GenericApiCore.o \
	CTGenericAPI/GenericConnection.o \
	CTGenericAPI/GenericMessage.o \
	CTServiceAPI.o \
	CTServiceAPICore.o \
	Request.o \
	Response.o

INCLUDE_DIRS = . ../../stlport453/stlport
CFLAGS_USER=-DEXTERNAL_DISTRO -DNAMESPACE=CTService -DUSE_TCP_LIBRARY

# # # # # # # # # # # # # # # # # # # # #

EMPTY_CHAR=
SPACE_CHAR=$(empty) $(empty)
COLON_CHAR=:

# # # # # # # # # # # # # # # # # # # # #

INCLUDE_DIR_LIST=$(subst $(COLON_CHAR),$(SPACE_CHAR),$(INCLUDE_PATH)) $(INCLUDE_DIRS) 
INCLUDE_FLAGS=$(addprefix -I,$(INCLUDE_DIR_LIST))
CFLAGS_DEBUG=$(INCLUDE_FLAGS) -D_DEBUG -g
CFLAGS_RELEASE=$(INCLUDE_FLAGS) -DNDEBUG -O2
#CFLAGS=$(CFLAGS_DEBUG) $(CFLAGS_USER) -D_GNU_SOURCE -Wall -Wno-unknown-pragmas
CFLAGS=$(CFLAGS_RELEASE) -D_GNU_SOURCE -Wall -Wno-unknown-pragmas

.SUFFIXES: .cpp .c

all: clean $(TARGET)

$(TARGET): $(OBJS)
	ld -r $(OBJS) -o $(TARGET)

clean:
	rm -f $(TARGET)
	rm -f $(OBJS)

.cpp.o:
	g++ $(CFLAGS) -c $< -o $*.o

.c.o:
	gcc $(CFLAGS) -c $< -o $*.o
