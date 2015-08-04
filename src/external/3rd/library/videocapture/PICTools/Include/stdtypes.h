/*.T(32)  STDTYPES.H    Standard Types for C Programs   */
// PICTools ProductVersion 2.00.567

/***************************************************************************\
*                                                                           *
* Copyright (C) 1993-2008 Pegasus Imaging Corporation  All rights reserved. *
*                                                                           *
\***************************************************************************/

#if !defined(_STDTYPES)
#define _STDTYPES

/***********************************************************************\
*                                                                       *
*   The following types are independent of the model:                   *
*                                                                       *
*   CHAR  / BYTE        - signed / unsigned 8-bit value                 *
*   SHORT / WORD        - signed / unsigned 16-bit value                *
*   LONG  / DWORD       - signed / unsigned 32-bit value                *
*                                                                       *
*   The following type is dependent on the model:                       *
*                                                                       *
*   INT   / UINT        - signed / unsigned value (16 or 32 bits)       *
*   BOOL                - signed int (FALSE = 0, TRUE = 1)              *
*                                                                       *
*   The enumeration type should NOT be used since some compilers require*
*   that this be an int, while others allow it to be 1-, 2-, or 4-bytes *
*   depending on the values used by the enumeration elements.           *
*                                                                       *
\***********************************************************************/



#include <stddef.h>
typedef ptrdiff_t PICPTRDIFFT;
typedef size_t    PICSIZET;



#if defined(WINDOWS)

    #pragma message ("WINDOWS defined")

    #pragma warning(disable:4115)
    #include <windows.h>
    #pragma warning(default:4115)

    #if !defined(__FLAT__) && ( defined(WIN32) || defined(WIN64) )
        #define __FLAT__
    #endif
    #define EXPORTWINAPI WINAPI
    #if defined(__FLAT__)
        #define DLLEXPORTWINAPI WINAPI
        #define PICHUGE
        #define PICFAR
        #if defined(PIC64)
            #define PICFARFN
            typedef DWORD_PTR PICINTPTRT;
            typedef DWORD PICXWORD;
            typedef LONG  PICXSHORT;
            typedef __int64 PICLONGLONG;
            typedef unsigned __int64 PICULONGLONG;
        #else
            #define PICFARFN __cdecl
            typedef DWORD  PICINTPTRT;
            typedef WORD   PICXWORD;
            typedef SHORT  PICXSHORT;
        #endif
    #else
        #define DLLEXPORTWINAPI WINAPI __loadds
        #define PICHUGE  __huge
        #define PICFAR   __far
        #define PICFARFN __far __cdecl
        typedef DWORD PICINTPTRT;
    #endif /* __FLAT__ */

#elif defined(OS2)

    #define INCL_DOS
    #define INCL_DOSPROCESS
    #include <os2.h>

    #if !defined(__FLAT__)
        #define __FLAT__
    #endif
    #if defined(__IBMC__) || defined(__IBMCPP__)
        #if !defined(WINAPI)
            #define WINAPI _System
        #endif
        #define EXPORTWINAPI _System
        #define DLLEXPORTWINAPI _System
    #else
        #if !defined(WINAPI)
            #define WINAPI _syscall
        #endif
        #define EXPORTWINAPI _syscall
        #define DLLEXPORTWINAPI _syscall
    #endif /* __IBMC__ */
    #define PICHUGE
    #define PICFAR
    #define PICFARFN
    typedef size_t PICINTPTRT;
    typedef unsigned short PICXWORD;
    typedef short PICXSHORT;

#elif defined(MACINTOSH) || defined(macintosh) || defined(__APPLE__)

    #if !defined(MACINTOSH)
        #define MACINTOSH
    #endif
    #if !defined(__FLAT__)
        #define __FLAT__
    #endif
    #define WINAPI
    #define EXPORTWINAPI
    #define DLLEXPORTWINAPI
    #define PICHUGE
    #define PICFAR
    #define PICFARFN
    #define __pascal
    #define __cdecl
    typedef size_t PICINTPTRT;
    typedef unsigned short PICXWORD;
    typedef short PICXSHORT;

#elif defined(__beos__)

    #if !defined(__FLAT__)
        #define __FLAT__
    #endif
    #define WINAPI
    #define DLLEXPORTWINAPI
    #define EXPORTWINAPI
    #define PICHUGE
    #define PICFAR
    #define PICFARFN
    #define __pascal
    #define __cdecl
    typedef size_t PICINTPTRT;
    typedef unsigned short PICXWORD;
    typedef short PICXSHORT;

#elif defined(__QNX__)

    #if !defined(__FLAT__)
        #define __FLAT__
    #endif
    #define WINAPI __stdcall
    #define DLLEXPORTWINAPI __stdcall
    #define EXPORTWINAPI __stdcall
    #define PICHUGE
    #define PICFAR
    #define PICFARFN
    #define __pascal
    typedef size_t PICINTPTRT;
    typedef unsigned short PICXWORD;
    typedef short PICXSHORT;

#elif defined(__unix__)

    #if !defined(__FLAT__)
        #define __FLAT__
    #endif
    #define WINAPI
    #define DLLEXPORTWINAPI
    #define EXPORTWINAPI
    #define PICHUGE
    #define PICFAR
    #define PICFARFN
    #define __pascal
    #define __cdecl
    typedef size_t PICINTPTRT;
    #if defined(PIC64)
        typedef unsigned int PICXWORD;
        typedef int PICXSHORT;
    #else
        typedef unsigned short PICXWORD;
        typedef short PICXSHORT;
    #endif


#else

    #if !defined(__FLAT__)
        #define __FLAT__
    #endif
    #define WINAPI
    #define DLLEXPORTWINAPI
    #define EXPORTWINAPI
    #define PICHUGE
    #define PICFAR
    #define PICFARFN
    typedef size_t PICINTPTRT;
    typedef unsigned short PICXWORD;
    typedef short PICXSHORT;

#endif




#if !defined(PIC_DONT_DEFINE_STDTYPES)

    /* following intended for internal use defining CPU-specific macros for unaligned access to
        words and dwords and for little-endian and big-endian access to words and dwords */

        /* following for endian match when alignment isn't an issue */
    #define _picgetW(x)          ( *(WORD*)(x) )
    #define _picgetDW(x)         ( *(DWORD*)(x) )

    #define _picsetW(x, w)       ( *(WORD*)(x) = (w) )
    #define _picsetDW(x, dw)     ( *(DWORD*)(x) = (dw) )

        /* else endian doesn't match or alignment is an issue */
    #define _picgetWle(x)       ( (WORD)( ((BYTE*)(x))[0] + ( ((BYTE*)(x))[1] << 8UL ) ) )
    #define _picgetWbe(x)       ( (WORD)( ( ((BYTE*)(x))[0] << 8UL ) + ((BYTE*)(x))[1] ) )
    #define _picgetDWle(x)  \
        ( (DWORD)( ((BYTE*)(x))[0] + ( ((BYTE*)(x))[1] << 8UL ) + ( ((BYTE*)(x))[2] << 16UL ) + ( ((BYTE*)(x))[3] << 24UL ) ) )
    #define _picgetDWbe(x)  \
        ( (DWORD)( ( ((BYTE*)(x))[0] << 24UL ) + ( ((BYTE*)(x))[1] << 16UL ) + ( ((BYTE*)(x))[2] << 8UL ) + ((BYTE*)(x))[3] ) )

    #define _picsetWle(x, w)     ( ((BYTE*)(x))[0] = (BYTE)(w), ((BYTE*)(x))[1] = (BYTE)( (w) >> 8UL ) )
    #define _picsetWbe(x, w)     ( ((BYTE*)(x))[0] = (BYTE)( (w) >> 8UL ), ((BYTE*)(x))[1] = (BYTE)(w) )
    #define _picsetDWle(x, dw)   \
        ( ((BYTE*)(x))[0] = (BYTE)(dw), ((BYTE*)(x))[1] = (BYTE)( (dw) >> 8UL ), \
          ((BYTE*)(x))[2] = (BYTE)( (dw) >> 16UL ), ((BYTE*)(x))[3] = (BYTE)( (dw) >> 24UL ) )
    #define _picsetDWbe(x, dw)   \
        ( ((BYTE*)(x))[0] = (BYTE)( (dw) >> 24UL ) , ((BYTE*)(x))[1] = (BYTE)( (dw) >> 16UL ), \
          ((BYTE*)(x))[2] = (BYTE)( (dw) >> 8UL ), ((BYTE*)(x))[3] = (BYTE)(dw) )



    #define SWAPW(w)    (((w) << 8) | (BYTE)((w) >> 8))
    #define SWAPD(d)    (((d) << 24) | (((d) & 0xFF00) << 8) |  (((d) & 0xFF0000) >> 8) | (BYTE)((d) >> 24))

    typedef signed char SBYTE;
    #define CHAR char
    #define INT int
    #define SHORT short
    #define PRIVATE static
    #define PUBLIC

    #if defined(WINDOWS)

        #define picIsBE             (0)
        #define picIsLE             (!picIsBE)

        #define picgetW(x)          _picgetW((x))
        #define picgetDW(x)         _picgetDW((x))
        #define picgetWle(x)        _picgetW((x))
        #define picgetDWle(x)       _picgetDW((x))
        #define picgetWbe(x)        _picgetWbe((x))
        #define picgetDWbe(x)       _picgetDWbe((x))

        #define picsetW(x, w)       _picsetW((x), (w))
        #define picsetDW(x, dw)     _picsetDW((x), (dw))
        #define picsetWle(x, w)     _picsetW((x), (w))
        #define picsetDWle(x, dw)   _picsetDW((x), (dw))
        #define picsetWbe(x, w)     _picsetWbe((x), (w))
        #define picsetDWbe(x, dw)   _picsetDWbe((x), (dw))

    #elif defined(OS2)

        typedef PICPTRDIFFT LPARAM;
        typedef unsigned short WORD;
        typedef unsigned long DWORD;

        #define picIsBE             (0)
        #define picIsLE             (!picIsBE)

        #define picgetW(x)          _picgetW((x))
        #define picgetDW(x)         _picgetDW((x))
        #define picgetWle(x)        _picgetW((x))
        #define picgetDWle(x)       _picgetDW((x))
        #define picgetWbe(x)        _picgetWbe((x))
        #define picgetDWbe(x)       _picgetDWbe((x))

        #define picsetW(x, w)       _picsetW((x), (w))
        #define picsetDW(x, dw)     _picsetDW((x), (dw))
        #define picsetWle(x, w)     _picsetW((x), (w))
        #define picsetDWle(x, dw)   _picsetDW((x), (dw))
        #define picsetWbe(x, w)     _picsetWbe((x), (w))
        #define picsetDWbe(x, dw)   _picsetDWbe((x), (dw))

    #elif defined(MACINTOSH)

        typedef PICPTRDIFFT LPARAM;
        typedef unsigned short WORD;
        typedef unsigned long DWORD;
        typedef signed int BOOL;
        typedef unsigned char BYTE;
        typedef signed long LONG;
        typedef unsigned int UINT;

        typedef WORD* LPWORD;
        typedef DWORD* LPDWORD;
        typedef BYTE* LPBYTE;
        typedef void* LPVOID;
        typedef const char* LPCSTR;
        typedef char* LPSTR;
        typedef WORD* PWORD;
        typedef BYTE* PBYTE;
        typedef DWORD* PDWORD;

        #if !defined(max)
            #define max(a,b) ((a) > (b) ? (a) : (b))
        #endif
        #if !defined(min)
            #define min(a,b) ((a) < (b) ? (a) : (b))
        #endif
        #if !defined(FALSE)
            #define FALSE (0)
            #define TRUE (1)
        #endif
        #define LOWORD(x) ((WORD)((x) & 0x0000FFFF))
        #define HIWORD(x) ((WORD)((x) >> 16))
        #define LOBYTE(x) ((BYTE)((x) & 0x00FF))
        #define HIBYTE(x) ((BYTE)((x) >> 8))

        #define picIsLE             (0)
        #define picIsBE             (!picIsLE)

        #define picgetW(x)          _picgetW((x))
        #define picgetDW(x)         _picgetDW((x))
        #define picgetWle(x)        _picgetWle((x))
        #define picgetDWle(x)       _picgetDWle((x))
        #define picgetWbe(x)        _picgetW((x))
        #define picgetDWbe(x)       _picgetDW((x))

        #define picsetW(x, w)       _picsetW((x), (w))
        #define picsetDW(x, dw)     _picsetDW((x), (dw))
        #define picsetWle(x, w)     _picsetWle((x), (w))
        #define picsetDWle(x, dw)   _picsetDWle((x), (dw))
        #define picsetWbe(x, w)     _picsetW((x), (w))
        #define picsetDWbe(x, dw)   _picsetDW((x), (dw))

    #else /* currently DOS4GW, __unix__, __beos__, __QNX__ */

        typedef PICPTRDIFFT LPARAM;
        typedef unsigned short WORD;
        #if defined(PIC64)
            typedef unsigned int DWORD;
            typedef signed int LONG;
        #else
            typedef unsigned long DWORD;
            typedef signed long LONG;
        #endif
        typedef signed int BOOL;
        typedef unsigned char BYTE;
        typedef unsigned int UINT;

        typedef WORD* LPWORD;
        typedef DWORD* LPDWORD;
        typedef BYTE* LPBYTE;
        typedef void* LPVOID;
        typedef const char* LPCSTR;
        typedef char* LPSTR;
        typedef WORD* PWORD;
        typedef BYTE* PBYTE;
        typedef DWORD* PDWORD;

        #if !defined(FALSE)
            #define FALSE (0)
            #define TRUE (1)
        #endif

        #if !defined(_MAX_PATH)
            #if defined(FILENAME_MAX)
                #define _MAX_PATH (FILENAME_MAX)
            #else
                #define _MAX_PATH (260)
            #endif
        #endif

        #if defined(__PAXPORT__)
            #undef __EABI__
            #define strtoul strtol
        #endif
        #if defined(__unix__) || defined(__beos__) || defined(__QNX__) || defined(__PAXPORT__)
            #if !defined(max)
                #define max(a,b) ((a) > (b) ? (a) : (b))
            #endif
            #if !defined(min)
                #define min(a,b) ((a) < (b) ? (a) : (b))
            #endif
        #endif
        #define wsprintf sprintf
        #define OutputDebugString printf
        #define LOWORD(x) ((WORD)((x) & 0x0000FFFF))
        #define HIWORD(x) ((WORD)((x) >> 16))
        #define LOBYTE(x) ((BYTE)((x) & 0x00FF))
        #define HIBYTE(x) ((BYTE)((x) >> 8))

        #if defined(__SPARC__) || defined(__PAXPORT__) || defined(_POWER)
            #define picIsLE             (0)
            #define picIsBE             (!picIsLE)
            #define picgetW(x)          _picgetWbe((x))
            #define picgetDW(x)         _picgetDWbe((x))
            #define picgetWle(x)        _picgetWle((x))
            #define picgetDWle(x)       _picgetDWle((x))
            #define picgetWbe(x)        _picgetWbe((x))
            #define picgetDWbe(x)       _picgetDWbe((x))

            #define picsetW(x, w)       _picsetWbe((x), (w))
            #define picsetDW(x, dw)     _picsetDWbe((x), (dw))
            #define picsetWle(x, w)     _picsetWle((x), (w))
            #define picsetDWle(x, dw)   _picsetDWle((x), (dw))
            #define picsetWbe(x, w)     _picsetWbe((x), (w))
            #define picsetDWbe(x, dw)   _picsetDWbe((x), (dw))
        #else /* assume intel-ordered */
            #define picIsBE             (0)
            #define picIsLE             (!picIsBE)
            #define picgetW(x)          _picgetW((x))
            #define picgetDW(x)         _picgetDW((x))
            #define picgetWle(x)        _picgetW((x))
            #define picgetDWle(x)       _picgetDW((x))
            #define picgetWbe(x)        _picgetWbe((x))
            #define picgetDWbe(x)       _picgetDWbe((x))

            #define picsetW(x, w)       _picsetW((x), (w))
            #define picsetDW(x, dw)     _picsetDW((x), (dw))
            #define picsetWle(x, w)     _picsetW((x), (w))
            #define picsetDWle(x, dw)   _picsetDW((x), (dw))
            #define picsetWbe(x, w)     _picsetWbe((x), (w))
            #define picsetDWbe(x, dw)   _picsetDWbe((x), (dw))
        #endif
    #endif

    /* runtime detection of endian and safe alignment access if above needs to be overriden for some reason
            (safe assuming that "\1" is WORD-aligned or else that the compiler is smart enough to evaluate
            it as a constant expression) */
    #if defined(__PIC_SAFE_RUNTIME_ENDIAN__)
        #undef picIsBE
        #undef picIsLE
        #undef picgetW
        #undef picgetDW
        #undef picgetWle
        #undef picgetDWle
        #undef picgetWbe
        #undef picgetDWbe
        #undef picsetW
        #undef picsetDW
        #undef picsetWle
        #undef picsetDWle
        #undef picsetWbe
        #undef picsetDWbe

        #define picIsLE             ( *(WORD*)"\1" == 1 )
        #define picIsBE             (!picIsLE)
        #define picgetW(x)          ( (WORD)( ( *(WORD*)"\1" == 1 ) ? _picgetWle((x)) : _picgetWbe((x)) ) )
        #define picgetDW(x)         ( (DWORD)( ( *(WORD*)"\1" == 1 ) ? _picgetDWle((x)) : _picgetDWbe((x)) ) )
        #define picgetWle(x)        _picgetWle((x))
        #define picgetDWle(x)       _picgetDWle((x))
        #define picgetWbe(x)        _picgetWbe((x))
        #define picgetDWbe(x)       _picgetDWbe((x))

        #define picsetW(x, w)       ( ( *(WORD*)"\1" == 1 ) ? _picsetWle((x), (w)) : _picsetWbe((x), (w)) )
        #define picsetDW(x, dw)     ( ( *(WORD*)"\1" == 1 ) ? _picsetDWle((x), (dw)) : _picsetDWbe((x), (dw)) )
        #define picsetWle(x, w)     _picsetWle((x), (w))
        #define picsetDWle(x, dw)   _picgetDWle((x), (dw))
        #define picsetWbe(x, w)     _picsetWbe((x), (w))
        #define picsetDWbe(x, dw)   _picsetDWbe((x), (dw))
    #endif

#endif /* !defined(PIC_DONT_DEFINE_STDTYPES) */

#if defined(USEPICCLIB) || defined(USEPICSETJMP)
    #if !defined(USEPICCLIB)
        #if !defined(USECLIBMALLOC)
            #define USECLIBMALLOC
        #endif
        #if !defined(USECLIBMISC)
            #define USECLIBMISC
        #endif
    #endif
    #include "picclib.h"
#endif

/* This would ordinarily be defined in bmp.h with RGBQUAD, but it needs to be defined for all platforms. */
/* bmp.h is not included for Windows platforms. */
/* The range of each value is always 0-65535, never 0-255 */
/* To convert to RGBQUAD, use C8.rgbBlue = (C16.rgbBlue + 128) / 257 */
/* To convert from RGBQUAD, use C16.rgbBlue = C8.rgbBlue * 257 */
/* For a faster operation and little loss of precision, use C8.rgbBlue = C16.rgbBlue / 256 */
/* When appropriate, rgbReserved will hold an alpha value (0=transparent, 65535=opaque) */
typedef struct {
        WORD    rgbBlue;
        WORD    rgbGreen;
        WORD    rgbRed;
        WORD    rgbReserved;
    } RGBQUAD16;

#endif /* !defined(_STDTYPES) */
