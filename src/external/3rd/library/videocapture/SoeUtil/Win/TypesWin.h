#ifndef SOEUTIL_TYPESWIN_H
#define SOEUTIL_TYPESWIN_H

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE: This file is NOT allowed to include any platform-specific header files (such as windows.h) as it will expose the entire 
//       library (and application) to such header files.  Avoiding this is a primary design objective of the platform abstraction 
//       layer.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// visual studio

#ifdef _DEBUG
    #define SOEUTIL_DEBUG
#endif

#define SOEUTIL_FORCEINLINE __forceinline
#define SOEUTIL_ALIGN(type) __declspec(align(8))    // visual studio does not support aligning to a secondary type, so align to 8 instead
#define SOEUTIL_ALIGN_4 __declspec(align(4))
#define SOEUTIL_ALIGN_8 __declspec(align(8))
#define SOEUTIL_ALIGN_16 __declspec(align(16))
#define SOEUTIL_ALIGNOF(type) __alignof(type)
#define SOEUTIL_NEW_ALIGNMENT 8             // minimum guaranteed alignment of the 'new' operator

#ifdef SOEUTIL_DEBUG
#define SOEUTIL_SUPPRESS_WARNING(b) SoeUtil::SystemInternal::SuppressWarning(b)
#else
#define SOEUTIL_SUPPRESS_WARNING(b) (b)
#endif


#ifndef SOEUTIL_ASSERT          // this allows application to override assert behavior with compiler settings
    #ifdef SOEUTIL_DEBUG
        #define SOEUTIL_ASSERT(_Expression) (void)( (!!(_Expression)) || (__debugbreak(), 1) )
        //#define SOEUTIL_ASSERT(x) if (!(x)) __debugbreak(); else ((void)0) 
        // #define SOEUTIL_ASSERT(x) if (!(x)) *((uint *)0) = 0x12345678; else ((void)0)
    #else
        #define SOEUTIL_ASSERT(x) ((void)0)
    #endif
#endif



namespace SoeUtil
{

    namespace SystemInternal
    {
        inline bool SuppressWarning(bool b) { return b; }
    }

    // compiler specific implementation so we can key off the RTTI status of the compilation unit to optimize things and avoid warnings
    template <typename T_NEW, typename T_OLD> T_NEW SafeDownCast(T_OLD old)
    {
        #ifdef _CPPRTTI
            T_NEW t = dynamic_cast<T_NEW>(old);
            SOEUTIL_ASSERT(old == NULL || t != NULL);
            return t;
        #else
            return static_cast<T_NEW>(old);
        #endif
    }

}   // namespace SoeUtil


#endif

