/****************************************************************************\
VeTime                      Neal Kettler

VeTime - Useful time/date class
\****************************************************************************/

#ifndef VETIME_HEADER
#define VETIME_HEADER

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>

#ifndef _WIN32
#include <unistd.h>
#include <netinet/in.h>
#include <sys/time.h>
#else
#include <sys/timeb.h>
#include <winsock.h>
#endif

#include <time.h>
#include <string.h>


class VeTime
{
public:

                VeTime();  // init to system time
                VeTime( const VeTime& other );
                VeTime( const time_t other );   // 1970-2038
                ~VeTime();

    void        addSeconds( int32 seconds );
    void        addMilliseconds( int32 milliseconds );

    bool        getTime( int& month, int& mday, int& year, int& hour, int& minute, int& second ) const;

    bool        setTime( int month, int mday, int year, int hour, int minute, int second );

    void        update();   // Update members sec & usec to system time
    
    static void sleep( int msec );

    int32       getDay( void ) const;    // Get days since year 0 
    int32       getMsec( void ) const;   // Get milliseconds into the day

    void        setDay( int32 day );
    void        setMsec( int32 msec );

    void        set( int32 newday, int32 newmsec );
    bool        parseDate( char* in );
    bool        formatTime( char* out, char* format );

    bool        getTimeval( struct timeval& tv ) const;

    time_t      getTime( void ) const;  // get in time_t format (like from time())

    // All of these may return -1 if the time is invalid
    int         getSecond( void ) const; // Second (0-60) (60 is for a leap second)
    int         getMinute( void ) const; // Minute (0-59)
    int         getHour( void ) const;   // Hour (0-23)
    int         getMDay( void ) const;   // Day of Month (1-31)
    int         getWDay( void ) const;   // Day of Week  (1-7)
    int         getYDay( void ) const;   // Day of Year  (1-366)  (366 = leap yr)
    int         getMonth( void ) const;  // Month (1-12)
    int         getYWeek( void ) const;  // Week of Year (1-53)
    int         getYear( void ) const;   // Year (e.g. 1997)

    // Modify the time components.  Return false if fail
    bool        setSecond( int32 sec );
    bool        setMinute( int32 min );
    bool        setHour( int32 hour );
    bool        setYear( int32 year );
    bool        setMonth( int32 month );
    bool        setMDay( int32 mday );

    void        normalize( void );  // move msec overflows to the day 

    // Compare two times
    int         compare( const VeTime& other ) const;

    // comparisons
    bool        operator ==( const VeTime& other ) const;
    bool        operator !=( const VeTime& other ) const;
    bool        operator  <( const VeTime& other ) const;
    bool        operator  >( const VeTime& other ) const;
    bool        operator <=( const VeTime& other ) const;
    bool        operator >=( const VeTime& other ) const;

    // assignments
    VeTime&      operator =( const VeTime& other );
    VeTime&      operator =( const time_t other );

    // signed
    VeTime&      operator +=( const VeTime& other );
    VeTime&      operator -=( const VeTime& other );
    VeTime       operator +( VeTime& other );
    VeTime       operator -( VeTime& other );

    VeTime&      operator +=( const time_t other );
    VeTime&      operator -=( const time_t other );
    VeTime       operator +( time_t other );
    VeTime       operator -( time_t other );

protected:
    int32       day_;    // days since Jan 1, 0
    int32       msec_;   // milliseconds  (thousandths of a sec)
};

#endif
