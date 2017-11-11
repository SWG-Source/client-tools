// Copyright 2007 Sony Online Entertainment, all rights reserved.
// Original author: Jeff Petersen

#ifndef SOEUTIL_TIME_H
#define SOEUTIL_TIME_H

#include "Types.h"
#include "StrongTypedef.h"
#include "String.h"

namespace SoeUtil
{
namespace Time
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The functions in this module are a collection of time related utility functions.
// All of these functions are thread-safe.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// real-time based time functions (value in seconds since 01/01/1970 UTC)
// note: calculating elapsed time from a 0 is allowed with seconds, since it has a known base value
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SOEUTIL_STRONG_TYPEDEF(int64, SecondsStamp);         // a value in seconds since 01/01/1970 UTC
const SecondsStamp cSecondsNull(0);

SecondsStamp GetSeconds();
int GetSecondsElapsed(SecondsStamp startTime);
int GetSecondsElapsed(SecondsStamp *pStartTime);    // pStartTime to current time
int GetSecondsElapsed(SecondsStamp startTime, SecondsStamp endTime);
SecondsStamp AdjustSeconds(SecondsStamp stamp, int delta);  // adjusts 'stamp' by 'delta', returning new stamp


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arbitrary millisecond based time functions
// note: a value in milliseconds, from some arbitrary time in the past (that may change from run to run)
// Guaranteed not to go backwards, skip forward, or wrap around during the course of the currently executing process.
// Cannot be compared to time stamps taken from other processes.
// note: calculating elapsed time from a 0 or null timestamp is considered a bug and will assert
// note: if elapsed time is greater than cIntMax, then those functions will return cIntMax. This equates to a diff of about 23 days.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SOEUTIL_STRONG_TYPEDEF(int64, MillisecondsStamp);
const MillisecondsStamp cMillisecondsNull(0);

MillisecondsStamp GetMilliseconds();                        // updates cached time
int GetMillisecondsElapsed(MillisecondsStamp startTime);
int GetMillisecondsElapsed(MillisecondsStamp *pStartTime);  // udpates pStartTime to current time
int GetMillisecondsElapsed(MillisecondsStamp startTime, MillisecondsStamp endTime);
MillisecondsStamp AdjustMilliseconds(MillisecondsStamp stamp, int delta);  // adjusts 'stamp' by 'delta', returning new stamp


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arbitrary high-resolution nanosecond based time functions
// note: calculating elapsed time from a 0 or null timestamp is considered a bug and will assert
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SOEUTIL_STRONG_TYPEDEF(int64, NanosecondsStamp);
const NanosecondsStamp cNanosecondsNull(0);

NanosecondsStamp GetNanoseconds();
int64 GetNanosecondsElapsed(NanosecondsStamp start);
int64 GetNanosecondsElapsed(NanosecondsStamp *pStartTime);
int64 GetNanosecondsElapsed(NanosecondsStamp startTime, NanosecondsStamp endTime);
NanosecondsStamp AdjustNanoseconds(NanosecondsStamp stamp, int delta);  // adjusts 'stamp' by 'delta', returning new stamp


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// date/time conversion functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct DateTime
{
    int year;       // year
    int month;      // month of year (1 to 12)
    int day;        // day of month (1 to 31)
    int hour;       // hour of day (0 to 23)
    int minute;     // minute of hour (0 to 59)
    int second;     // second of minute (0 to 59)
};

// converts 'stamp' into date time format (either utc time or local time depending on if timezone/dst adjustment is requested)
void ConvertSecondsToDateTime(SecondsStamp stamp, DateTime *dest, bool adjustForTimezone);
SecondsStamp ConvertDateTimeToSeconds(const DateTime *source, bool adjustForTimezone);

// converts DateTime into an ISO-8601 formatted string (assumes DateTime struct is UTC time zone always)
void ConvertDateTimeToString(const DateTime *source, SoeUtil::String *dest);
void ConvertStringToDateTime(const SoeUtil::String *source, DateTime *dest);
void ConvertStringToDateTime(const char *source, DateTime *dest);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// elapsed time conversion functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct ElapsedTime
{
    int days;        // 0 to ?
    int hours;       // 0 to 23
    int minutes;     // 0 to 59
    int seconds;     // 0 to 59
};
void ConvertSecondsToElapsedTime(int elapsed, ElapsedTime *dest);
int ConvertElapsedTimeToSeconds(ElapsedTime *source);


////////////////////////////////////////////////////////////////////////////////
// inline implementations (of portable functions)
////////////////////////////////////////////////////////////////////////////////

inline int GetSecondsElapsed(SecondsStamp startTime)
{
    return((int)(GetSeconds().AsBase() - startTime.AsBase()));
}

inline int GetSecondsElapsed(SecondsStamp *pStartTime)
{
    SecondsStamp t = *pStartTime;
    *pStartTime = GetSeconds();
    return((int)(pStartTime->AsBase() - t.AsBase()));
}

inline int GetSecondsElapsed(SecondsStamp startTime, SecondsStamp endTime)
{
    return((int)(endTime.AsBase() - startTime.AsBase()));
}

inline SecondsStamp AdjustSeconds(SecondsStamp stamp, int delta)
{
    return SecondsStamp(stamp.AsBase() + delta);
}

inline int GetMillisecondsElapsed(MillisecondsStamp startTime)
{
    SOEUTIL_ASSERT(startTime.AsBase() != 0);       // error: diffing time against a 0 is a bug
    int64 diff = GetMilliseconds().AsBase() - startTime.AsBase();
    return(diff > cIntMax ? cIntMax : (int)diff);
}

inline int GetMillisecondsElapsed(MillisecondsStamp *pStartTime)
{
    SOEUTIL_ASSERT(pStartTime->AsBase() != 0);       // error: diffing time against a 0 is a bug
    MillisecondsStamp t = *pStartTime;
    *pStartTime = GetMilliseconds();
    int64 diff = pStartTime->AsBase() - t.AsBase();
    return(diff > cIntMax ? cIntMax : (int)diff);
}

inline int GetMillisecondsElapsed(MillisecondsStamp startTime, MillisecondsStamp endTime)
{
    SOEUTIL_ASSERT(startTime.AsBase() != 0);       // error: diffing time against a 0 is a bug
    int64 diff = endTime.AsBase() - startTime.AsBase();
    return(diff > cIntMax ? cIntMax : (int)diff);
}

inline MillisecondsStamp AdjustMilliseconds(MillisecondsStamp stamp, int delta)
{
    SOEUTIL_ASSERT(stamp.AsBase() != 0);          // error: adding time to 0 stamp is a bug
    return MillisecondsStamp(stamp.AsBase() + delta);
}

inline int64 GetNanosecondsElapsed(NanosecondsStamp startTime)
{
    SOEUTIL_ASSERT(startTime.AsBase() != 0);       // error: diffing time against a 0 is a bug
    return((int64)(GetNanoseconds().AsBase() - startTime.AsBase()));
}

inline int64 GetNanosecondsElapsed(NanosecondsStamp *pStartTime)
{
    SOEUTIL_ASSERT(pStartTime->AsBase() != 0);       // error: diffing time against a 0 is a bug
    NanosecondsStamp t = *pStartTime;
    *pStartTime = GetNanoseconds();
    return((int64)(pStartTime->AsBase() - t.AsBase()));
}

inline int64 GetNanosecondsElapsed(NanosecondsStamp startTime, NanosecondsStamp endTime)
{
    SOEUTIL_ASSERT(startTime.AsBase() != 0);       // error: diffing time against a 0 is a bug
    return((int64)(endTime.AsBase() - startTime.AsBase()));
}

inline NanosecondsStamp AdjustNanoseconds(NanosecondsStamp stamp, int delta)
{
    SOEUTIL_ASSERT(stamp.AsBase() != 0);          // error: adding time to 0 stamp is a bug
    return NanosecondsStamp(stamp.AsBase() + delta);
}

inline void ConvertSecondsToElapsedTime(int elapsed, ElapsedTime *dest)
{
    dest->days = elapsed / 86400;   //86400 seconds / day
    elapsed %= 86400;
    dest->hours = elapsed / 3600;   //3600 seconds / hour
    elapsed %= 3600;
    dest->minutes = elapsed / 60;
    dest->seconds = elapsed % 60;
}

inline int ConvertElapsedTimeToSeconds(ElapsedTime *source)
{
    return ( (source->days * 86400) + (source->hours * 3600) + (source->minutes * 60) + source->seconds );
}

inline void ConvertDateTimeToString(const DateTime *source, SoeUtil::String *dest)
{
    dest->Format("%04d-%02d-%02dT%02d:%02d:%02dZ", source->year, source->month, source->day, source->hour, source->minute, source->second);
}

inline void ConvertStringToDateTime(const SoeUtil::String *source, DateTime *dest)
{
    ConvertStringToDateTime(source->AsRead(), dest);
}


}   // namespace Time
}   // namespace SoeUtil

#endif

