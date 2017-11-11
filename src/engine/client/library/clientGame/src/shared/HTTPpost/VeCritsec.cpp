#include "clientGame/FirstClientGame.h"
#include "VeCritsec.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sched.h>
#endif


void VeCritsec::yield_thread( void )
{
#ifdef _WIN32            
    Sleep(0);   // yield the thread
#else
    sched_yield();
#endif         
}


