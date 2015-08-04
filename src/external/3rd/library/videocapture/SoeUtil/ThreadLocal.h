// Copyright 2007 Sony Online Entertainment, all rights reserved.
// Author: Jeff Petersen

#ifndef SOEUTIL_THREADLOCAL_H
#define SOEUTIL_THREADLOCAL_H

#include "Types.h"

namespace SoeUtil
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Thread local storage allows an application to store data whereby each thread in the process has their own instance
// of the stored valued.  For example, you could create a global variable of type ThreadLocalInt and then every
// thread that looked at that global value would get their own version of it, even though they would all access it
// through the exact same symbolic name.  For convenience, an int, float, and pointer version of the thread local
// storage concept are provided.  Values are guaranteed to be initialized to 0 on the first access by any thread
// in the process.
//
// Constructing one of these classes causes the class to allocate space for the value in the thread-local-storage
// segment of the thread-context header.  The space available in this segment is VERY limited and varies from
// platform to platform.  Windows NT and earlier for example only have enough space for 64 values to be allocated.
// Windows 2000 and later have enough space for around 1000 values to be allocated.  See platform specific
// implementations for more details on these limits.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ThreadLocalInt
{
    public:
        ThreadLocalInt();
        ~ThreadLocalInt();
        int Get() const;
        int Set(int rhs);

    private:
        byte m_data[8];
};

class ThreadLocalUint
{
    public:
        ThreadLocalUint();
        ~ThreadLocalUint();
        uint Get() const;
        uint Set(uint rhs);

    private:
        byte m_data[8];
};

class ThreadLocalVoid
{
    public:
        ThreadLocalVoid();
        ~ThreadLocalVoid();
        void *Get() const;
        void *Set(void *rhs);

    private:
        byte m_data[8];
};

class ThreadLocalFloat
{
    public:
        ThreadLocalFloat();
        ~ThreadLocalFloat();
        float Get() const;
        float Set(float rhs);

    private:
        byte m_data[8];
};

template<typename T> class ThreadLocalPointer
{
    public:
        ThreadLocalPointer() {}
        ~ThreadLocalPointer() {}
        T *Get() const { return (T *)m_pointer.Get(); }
        T *Set(T *rhs) { return (T *)m_pointer.Set(rhs); }

    private:
        ThreadLocalVoid m_pointer;
};


}   // namespace SoeUtil

#endif

