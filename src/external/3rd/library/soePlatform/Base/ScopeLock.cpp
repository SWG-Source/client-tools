#include "ScopeLock.h"

namespace Base
{
    CScopeLock::CScopeLock(CMutex& mutex) :
        mMutex(&mutex)
    {
        mMutex->Lock();
    }

    CScopeLock::CScopeLock(CScopeLock& lock) :
        mMutex(lock.mMutex)
    {
        mMutex->Lock();
    }

    CScopeLock::~CScopeLock()
    {
        mMutex->Unlock();
    }
}

