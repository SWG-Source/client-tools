// ScopeLock.h: interface for the CScopeLock class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SCOPELOCK_H
#define SCOPELOCK_H

#include "Mutex.h"

namespace Base
{
    class CScopeLock  
    {
        public:
	        CScopeLock(CMutex& mutex);
	        CScopeLock(CScopeLock& lock);
	        virtual ~CScopeLock();

        private:
            CMutex *mMutex;
    };
}
#endif // SCOPELOCK_H
