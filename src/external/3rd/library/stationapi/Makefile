#========================================================
# clear default build commands
#========================================================
.SUFFIXES:

#========================================================
# include path
#========================================================
INC_DIR = -I. \
	-I../RDP/include \
	-I../RDP/dbgutil \
	-I../RDP/rdp \
	-I../crypt \

#========================================================
# link libraries
#========================================================
LIBS =  -L../lib \
	-lm \
	-lvcrypt \
	-lrdp -ldbgutil \
	-lpthread \
#	-lnsl -lsocket -lpthread -lposix4 \

#========================================================
# debug / release flags
#========================================================
#RELEASE_FLAGS = -O3 -DNDEBUG
DEBUG_FLAGS = -g -fasm -D_DEBUG

#========================================================
# user flags
#========================================================
USER_FLAGS = -DGNU -D_REENTRANT

#========================================================
# set default build commands
#========================================================
CFLAGS = $(INC_DIR) $(RELEASE_FLAGS) $(DEBUG_FLAGS) $(USER_FLAGS)
CPPFLAGS =
#LINK_SO = g++ -G $(CFLAGS)
LINK_SO = ld -shared

%.o : %.c
	g++ -c -x c $(CFLAGS) $< 

%.o : %.cpp
	g++ -c -x c++ $(CFLAGS) $(CPPFLAGS) $< 

#========================================================
# project
#========================================================
SRCS=\
    PackClass.cpp \
    stationapi.cpp \
    stationapilist.cpp \
    stationrequest.cpp \
    StationAPISession.cpp \
    extend_rdp.c \
    order.cpp \

OBJS= \
    PackClass.o \
    stationapi.o \
    stationapilist.o \
    stationrequest.o \
    StationAPISession.o \
    extend_rdp.o \
    order.o \

#========================================================
# output
#========================================================
all: stationapidemo

stationapidemo : stationapidemo.o libstationapi.so
	g++ -g -o $@ $^ $(LIBS) 

libstationapi.so : $(OBJS)
	$(LINK_SO) -o $@ $^ 

libstationapi.a : $(OBJS)
	ar -r $@ $^

clean:
	rm -f *.o libstationapi.a libstationapi.so stationapidemo
	rm -f *.dsw *.dsp *.dep *.ncb *.plg *.opt *.mak

depend:
	g++ -MM $(CFLAGS) $(SRCS)


