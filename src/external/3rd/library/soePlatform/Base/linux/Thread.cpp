////////////////////////////////////////
//  Thread.cpp
//
//  Purpose:
// 	    1. Implementation of the CThread class.
//
//  Revisions:
// 	    07/10/2001  Created                                    
//

#include <pthread.h>
#include "./Thread.h"

namespace Base
{

    void *threadProc(void *threadPtr)
	{
        CThread &thread = *((CThread*)threadPtr);
	    thread.mThreadActive = true;
        thread.ThreadProc();
        thread.mThreadActive = false;

        return 0;
	}

    CThread::CThread()
    {
        mThreadID = 0;
        mThreadActive = false;
        mThreadContinue = false;
    }

    CThread::~CThread()
    {
        StopThread();
    }

    void CThread::StartThread()
	{
	    mThreadContinue = true;
        pthread_create(&mThreadID,0,threadProc,this);
        while (!IsThreadActive())
            Base::sleep(1);
	}

    int32 CThread::StopThread(int timeout)
	{
        timeout += time(0);

        mThreadContinue = false;
        while (mThreadActive && time(0)<timeout)
            sleep(1);
        if (mThreadActive)
        {
            mThreadActive = false;
            return eSTOP_TIMEOUT;
        }
        return eSTOP_SUCCESS;
	}

}

