#ifndef SOEUTIL_TYPES_H
#define SOEUTIL_TYPES_H

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2006 Sony Online Entertainment, all rights reserved.
// Original author: Jeff Petersen
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <sys/types.h>          // this will pull in definitions for size_t and such that are needed for defining things like operator new
#include <cstring>

#ifdef WIN32
    #include "Win/TypesWin.h"
#endif

namespace SoeUtil { namespace Math { }}
namespace SoeMath = SoeUtil::Math;         // namespace alias for convenience

namespace SoeUtil
{

// current definitions below will work as-is on all currently supported platforms.  Conditional compilation can be used in this
// header file for situations where that is not the case.
    typedef unsigned int uint;      // shorthand, will ALWAYS be 'unsigned int' on all platforms, regardless of size
    typedef unsigned char byte;

    typedef signed char int8;
    typedef signed short int16;
    typedef signed int int32;
    typedef signed long long int64;

    typedef unsigned char uint8;
    typedef unsigned short uint16;
    typedef unsigned int uint32;
    typedef unsigned long long uint64;

    const int cVersion = 101;

    // note: these are platform neutral, since bit-size is explicit
    const int8 cInt8Max = 0x7f;
    const int8 cInt8Min = -cInt8Max - 1;
    const int16 cInt16Max = 0x7fff;
    const int16 cInt16Min = -cInt16Max - 1;
    const int32 cInt32Max = 0x7fffffff;
    const int32 cInt32Min = -cInt32Max - 1;
    const int64 cInt64Max = 0x7fffffffffffffffLL;
    const int64 cInt64Min = -cInt64Max - 1;
    const uint8 cUint8Max = 0xff;
    const uint16 cUint16Max = 0xffff;
    const uint32 cUint32Max = 0xffffffff;
    const uint64 cUint64Max = 0xffffffffffffffffULL;

    // note: these are platform dependent and will have to be ifdef to something else on some platforms
    const int cIntMax = cInt32Max;
    const int cIntMin = cInt32Min;
    const uint cUintMax = cUint32Max;
}


// Use these Check macros to validate certain things at COMPILE time
// Example: validate that the size of a table matches the count of an enumeration used to index it
// Use SOEUTIL_CHECKSTATIC_NONBODY when you are not within the scope of a function, such as in class decls or header files
// sizeof does not work inside class decls, so you can't verify the size of your class (or a header) using SOEUTIL_CHECKSIZE.  Do that in your constructor
#define SOEUTIL_CHECKSTATIC(expr,msg) typedef char  ERROR_##msg[1][(expr)]
#define SOEUTIL_CHECKSTATIC_NONBODY(expr,msg) static inline void ERROR_##msg() { SOEUTIL_CHECKSTATIC(expr,msg); }
#define SOEUTIL_CHECKSTATIC_WARNING(expr,msg) static const bool ERROR_##msg = ( ((expr)!=0) - 1 )
#define SOEUTIL_CHECKSIZE(param,match) SOEUTIL_CHECKSTATIC(sizeof(param)==sizeof(match),SizeMismatch)
#define SOEUTIL_CHECKVALUE(val1,val2,msg) template <int> struct soeutil_checkvalue_##msg; template <> struct soeutil_checkvalue_##msg<val2> {}; void soeutil_checkvalue_test_##msg() { soeutil_checkvalue_##msg<val1>(); }
#define SOEUTIL_CHECKTYPESIZE(typename,size) SOEUTIL_CHECKVALUE(sizeof(typename),size,typename##_SizeChanged)

#define SOEUTIL_DECLARE_NOCOPY(ClassName) private: ClassName(const ClassName &); ClassName &operator=(const ClassName &)

#define SOEUTIL_INT_ALWAYS_32       // this causes the library to assume 'int' is 32 bits in size (mainly used to optimize serialization routines)

#ifdef SOEUTIL_DEBUG
    #define SOEUTIL_MEMORY_INIT(p, len) std::memset((p), 0xdd, (len))
    #define SOEUTIL_MEMORY_INVALIDATE(p, len) std::memset((p), 0xde, (len))
#else
    #define SOEUTIL_MEMORY_INIT(p, len) ((void)0)
    #define SOEUTIL_MEMORY_INVALIDATE(p, len) ((void)0)
#endif




#endif  // SOEUTIL_TYPES_H

