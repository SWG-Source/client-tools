////////////////////////////////////////
//  Platform.h
//
//  Purpose:
// 	    1. Include relevent system headers that are platform specific.
//      2. Declair global platform specific functionality.
//      3. Include primative type definitions
//
//  Global Functions:
// 	    getTimer() : Return the current high resolution clock count.
//      getTimerFrequency() : Return the frequency of the high resolution clock.
//      sleep() : Voluntarily relinquish timeslice of the calling thread for a 
//          specified number of milliseconds.
//      strlwr() : Alters the contents of a string, making it all lower-case.
//
//  Revisions:
// 	    07/10/2001  Created                                    
//

#ifndef	BASE_LINUX_PLATFORM_H
#define	BASE_LINUX_PLATFORM_H

#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/errno.h>
#include <pthread.h>
#include <resolv.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "Types.h"

namespace Base
{
int64 getTimer(void);
int64 getTimerFrequency(void);
void sleep(uint32 ms);

inline int64 getTimer(void)
    {
	int64	t;
	struct timeval	tv;
	
	gettimeofday(&tv, 0);
	t = tv.tv_sec;
	t = t * 1000000;
	t += tv.tv_usec;
	return t;
    }

inline int64 getTimerFrequency(void)
    {
	int64	f = 1000000;
	return f;
    }

inline void sleep(uint32 ms)
    {
	usleep(static_cast<unsigned long>(ms * 1000));
    }

void _strlwr(char * s);
void _strupr(char * s);

}

#endif // BASE_LINUX_PLATFORM_H

