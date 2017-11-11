// Copyright 2009 Sony Online Entertainment, all rights reserved.
// Original author: Jeff Petersen
#ifndef SOEUTIL_MEMORY_H
#define SOEUTIL_MEMORY_H

#include "ThreadLocal.h"
#include <new>


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The basic idea is to allow hooking of the SoeUtil memory allocations that occur.  The application call 
// SoeUtil::SetMemoryHandler to point SoeUtil to use the specified MemoryHandler derived class.  All allocations 
// by SoeUtil that occur then get routed to the handler.
//
// Normally SetMemoryHandler only changes the handler for the current thread-context.  Applications can use
// SetGlobalMemoryHandler to change the MemoryHandler used for all thread contexts.  Thread-specific handlers take
// precedence over the global handler, which takes precedence over going directly to the built-in heap.  By default
// there are no handlers installed and everything goes directly to the built-in heap.
//
// Not all memory activity inside of SoeUtil routes through the MemoryHandler.  In particular, the DeleteAll
// convenience function on embedded containers, the algorithm ListDeleteAll, and the RefCounted::RefDestroySelf
// function do not go through the handler to free things, but instead always simply call 'delete' since most of the
// time these are allocations that were done by the application directly.  Serialization and Unserialization of 
// embedded containers also do not go through the memory handler, for similar reasons.
//
// The application is free to also hook through the SoeUtil memory handler by doing allocations via the
// SoeUtil::Alloc and SoeUtil::Free functions.
//
// Warning: It is not safe to delete an object via its base-class pointer by explicitly calling the destructor and
// then calling SoeUtil::Free on the pointer to free the memory.  The base-class pointer is not guaranteed to point
// to the beginning of the originally allocated memory-block in situations where multiple-inheritance has been used.
// This is partly the reason why SoeUtil makes no attempt to have DeleteAll and Release functionality go through its
// custom allocator.  The only safe way to destruct an object via a pointer to it base class is via the 'delete'
// operator.  If these things must be routed to a custom allocator, then overloading the new/delete operator on a
// per class basis (or the global new/delete operator) would be required.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


namespace SoeUtil
{


class MemoryHandler
{
    public:
        // if this handler is set as the global handler for all thread contexts, then these functions need to be thread-safe.
        virtual void *OnAlloc(int bytes) = 0;
        virtual void OnFree(void *p) = 0;
};

// These two calls set/get the memory handler for the global-context
// It is not safe to change the global handler if other threads are currently using the heap/SoeUtil, since they may get mismatched alloc/free calls
void SetGlobalMemoryHandler(MemoryHandler *handler);
MemoryHandler *GetGlobalMemoryHandler();

// These two calls set/get the memory handler for this thread-context
void SetMemoryHandler(MemoryHandler *handler);
MemoryHandler *GetMemoryHandler();

void *Alloc(int bytes);     // returns NULL on failure
void Free(void *p);


class MemoryHandlerGuard
{
    public:
        MemoryHandlerGuard(MemoryHandler *handler)
        {
            m_oldHandler = GetMemoryHandler();
            SetMemoryHandler(handler);
        }

        ~MemoryHandlerGuard()
        {
            SetMemoryHandler(m_oldHandler);
        }

    protected:
        MemoryHandler *m_oldHandler;
};

// deriving from this base class will cause derived classes to route through the SoeUtil::Alloc and SoeUtil::Free functions
class MemoryBase
{
    public:
        static void *operator new(size_t size)                                      { void *p = Alloc((int)size); if (p == NULL) { throw std::bad_alloc(); } return p; }
        static void *operator new[](size_t size)                                    { void *p = Alloc((int)size); if (p == NULL) { throw std::bad_alloc(); } return p; }
        static void *operator new(size_t size, const std::nothrow_t &) throw()      { return Alloc((int)size); }
        static void *operator new(size_t /*size*/, void *p) throw()                     { return p; }
        static void *operator new[](size_t size, const std::nothrow_t &) throw()    { return Alloc((int)size); }
        static void *operator new[](size_t /*size*/, void *p) throw()                   { return p; }

        static void operator delete(void *p) throw()                                { Free(p); }
        static void operator delete[](void *p) throw()                              { Free(p); }
        static void operator delete(void *p, const std::nothrow_t &) throw()        { Free(p); }
        static void operator delete(void * /*p*/, void *) throw()                        {}
        static void operator delete[] (void *p, const std::nothrow_t &) throw()     { Free(p); }
        static void operator delete[] (void * /*p*/, void *) throw()                     {}
};


////////////////////////////////////////////////////////////////////////////////////////////////////////
// inline implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace SystemInternal
{
    inline MemoryHandler *&GlobalMemoryHandler()
    {
        static MemoryHandler *s_memoryHandler = NULL;
        return s_memoryHandler;
    }

    inline SoeUtil::ThreadLocalPointer<MemoryHandler> &GlobalTlsMemoryHandler()
    {
        static SoeUtil::ThreadLocalPointer<MemoryHandler> s_tls_memoryHandler;
        return s_tls_memoryHandler;
    }
}

inline void *Alloc(int bytes)
{
    if (SystemInternal::GlobalTlsMemoryHandler().Get() != NULL)
    {
        return SystemInternal::GlobalTlsMemoryHandler().Get()->OnAlloc(bytes);
    }
    else if (SystemInternal::GlobalMemoryHandler() != NULL)
    {
        return SystemInternal::GlobalMemoryHandler()->OnAlloc(bytes);
    }
    else
    {
        return new(std::nothrow) byte[bytes];
    }
}

inline void Free(void *p)
{
    if (p != NULL)
    {
        if (SystemInternal::GlobalTlsMemoryHandler().Get() != NULL)
        {
            SystemInternal::GlobalTlsMemoryHandler().Get()->OnFree(p);
        }
        else if (SystemInternal::GlobalMemoryHandler() != NULL)
        {
            SystemInternal::GlobalMemoryHandler()->OnFree(p);
        }
        else
        {
            delete[] (byte *)p;
        }
    }
}

inline void SetGlobalMemoryHandler(MemoryHandler *handler)
{
    SystemInternal::GlobalMemoryHandler() = handler;
}

inline MemoryHandler *GetGlobalMemoryHandler()
{
    return SystemInternal::GlobalMemoryHandler();
}

inline void SetMemoryHandler(MemoryHandler *handler)
{
    SystemInternal::GlobalTlsMemoryHandler().Set(handler);
}

inline MemoryHandler *GetMemoryHandler()
{
    return SystemInternal::GlobalTlsMemoryHandler().Get();
}


}   // namespace


#endif

