#################################################################
## compiler data

CC=g++
LD=g++
CFLAGS_RELEASE=-D_DEBUG -D_REENTRANT -D_GNU_SOURCE
CFLAGS=-g ${CFLAGS_RELEASE}

#################################################################
## library data

LIBS=MonApi UdpLibrary
LDFLAGS=-L../../lib/linux-debug $(addprefix -l,$(LIBS))
LDFLAGS_RELEASE=-L../../lib/linux-release $(addprefix -l,$(LIBS))

#################################################################
##  object data

OBJECTS=MonAPITest.o
INCLUDE=-I. -I.. -I../..

#################################################################
## target data

TARGET=montest

${TARGET}: ${OBJECTS}
	${CC} -o $@ ${OBJECTS} ${LDFLAGS} 
#	${CC} -o $@ ${OBJECTS} ${LDFLAGS_RELEASE} 

clean:
	rm *.o ${TARGET}

all: clean ${TARGET}
#################################################################
## suffix data

.SUFFIXES : .o .cpp

.cpp.o :
	${CC} ${INCLUDE} ${CFLAGS} -c $<
#	${CC} ${INCLUDE} ${CFLAGS_RELEASE} -c $<
