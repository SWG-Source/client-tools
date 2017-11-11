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
//
//  Revisions:
// 	    07/10/2001  Created                                    
//

#ifndef	BASE_WIN32_PLATFORM_H
#define	BASE_WIN32_PLATFORM_H

#include <string.h>
#include <memory.h>
#include <winsock2.h>
#include "Types.h"

namespace Base
{

int64 getTimer(void);
int64 getTimerFrequency(void);

inline int64 getTimer(void)
    {
    int64 result;
    if (!QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&result)))
        result = 0;
    return result;
    }

inline int64 getTimerFrequency(void)
    {
    int64 result;
    if (!QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER *>(&result)))
        result = 0;
    return result;
    }

inline void sleep(uint32 ms)
    {
	Sleep(ms);
    }

}
#endif	BASE_WIN32_PLATFORM_H