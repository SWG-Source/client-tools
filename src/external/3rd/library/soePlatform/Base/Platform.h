#ifndef	BASE_PLATFORM_H
#define	BASE_PLATFORM_H

#ifdef WIN32

    #include "./win32/Platform.h"

#elif linux

    #include "./linux/Platform.h"

#elif sparc

    #include "./solaris/Platform.h"

#else

    #error /Base/Platform.h: Undefine platform type

#endif

#endif	//  BASE_PLATFORM_H

