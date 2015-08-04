#include "Statistics.h"

namespace Base
{

    CStatisticTimer::CStatisticTimer(bool running) :
        mTotal(0),
        mStart(0),
        mRunning(running)
    {
    }

    double CStatisticTimer::GetTime()
    {
        int64 total = mTotal;

        if (mRunning)
            total += getTimer()-mStart;

        return (double)total/getTimerFrequency();    
    }

    uint64 CStatisticTimer::GetFraction(uint32 fraction)
    {
        int64 total = mTotal;

        if (mRunning)
            total += getTimer()-mStart;

        if (fraction == 0)
            return total;
        else
            return total/(getTimerFrequency()/fraction);    
    }

}
