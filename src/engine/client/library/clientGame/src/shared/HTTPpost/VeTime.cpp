/****************************************************************************\

VeTime                    Neal Kettler

VeTime - Useful time/date class
\****************************************************************************/

#include "clientGame/FirstClientGame.h"
#include <ctype.h>
#include <time.h>
#include "VeTime.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock.h>
#else
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#endif



static char* DAYS[] =
{
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static char* FULLDAYS[] =
{
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

static char* MONTHS[] =
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static char* FULLMONTHS[] =
{
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
};


#define IS_LEAP(y) ((y) % 4) == 0 && (! ((y) % 100) == 0 || ((y) % 400) == 0)
#define LEAPS_THRU_END_OF(y) ((y) / 4 - (y) / 100 + (y) / 400)


/////////////// Utility functions ///////////////////

//
// Return the daycount since year 0 for the specified date.
// month = 1-12, day = 1-31 year = 0...
//
static int32 Get_Day( int month, int day, int year )
{
    time_t days;

    static int DaysAtMonth[] =
    {
        0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
    };

    /* Add up number of straight days + number of leap days + days */
    /* up to the month + the day of month. */

    days = ( year * 365 ) +
           ( year / 4 ) -
           ( year / 100 ) +
           ( year / 400 ) +
           DaysAtMonth[month - 1] +
           day;

    /* If we haven't hit Feb 29, and this is a leap year, we need to */
    /* subtract out the leap day that was added above for this year */

    if ( month < 3 && IS_LEAP( year ) )
        --days;
    return( static_cast<int32>(days) );
}



//
// Get the year from a daycount since year 0
// Also get the daycount since the start of the year
//
// Ayecarumba what a pain in the ass!
//
static bool Get_Date_From_Day( int32 days,  int& year,  int& yday )
{
    register long int y;

    if ( days <= 365 )
    {
        year = 0;
        yday = days + 1;  // 1 based
        return( true );
    }

    y = 1;
    days -= 365;

    days--;  // zero based

    //
    // As far as I can tell there's no non-iteritive way to
    //   do this...
    //
    while ( days < 0 || days >= ( IS_LEAP( y ) ? 366 : 365 ) )
    {
        /* Guess a corrected year, assuming 365 days per year.  */
        long int yg = y + days / 365 - ( days % 365 < 0 );

        /* Adjust DAYS and Y to match the guessed year.  */
        days -= ( ( yg - y ) * 365 + LEAPS_THRU_END_OF( yg - 1 ) - LEAPS_THRU_END_OF( y - 1 ) );
        y = yg;
    }
    year = y;
    yday = days + 1;  // 1 based
    return( true );
}


//
// Get the max day of a given month in a given year
//
int Max_Day( int month, int year )
{
    static char dayTable[2][13] =
    {
        {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
    };
    bool isleap = IS_LEAP( year );
    return( dayTable[isleap][month] );
}



/**********************************************************
int main(int argc, char *argv[])
{
  VeTime wtime;
  int month,mday,year,hour,minute,second,day;
  int i;
  int dayoffset;
  int yr;
  int yday;
  int ydaycount;

  wtime.set(719528,0);

***********************************************
  for (year=0; year<10000; year++)
  {
    ydaycount=1;
    for (month=1; month<=12; month++)
    {
      for (day=1; day<=Max_Day(month,year); day++)
      {
        dayoffset=Get_Day(month,day,year);
        assert (GetDateFromDay(dayoffset,yr,yday)==true);

        //printf("Yday=%d YdayCount=%d\n",yday,ydaycount);

        if (yr!=year)
        {
          printf("MO=%d  DAY=%d  YEAR=%d  YR=%d\n",month,day,year,yr);
          assert(0);
        }
        if (yday != ydaycount)
        {
          printf("MO=%d  DAY=%d  YEAR=%d  YR=%d\n",month,day,year,yr);
          printf("Yday=%d YdayCount=%d\n",yday,ydaycount);
          assert(0);
        }
        ydaycount++;
      }
    } 
    printf("(%d) ",year);
  }
***************************************

  ///////wtime.addSeconds((60*60*24)-(60*60*8));
  ////////wtime.addSeconds(-(60*60*8));  // timezone delta

  dayoffset=Get_Day(1,1,1970);
  printf("DAYOFFSET = %d\n",dayoffset);


  wtime.getTime(month, mday, year, hour, minute, second);
  printf("\n%s %d  %d  %d:%02d:%02d\n\n",
    MONTHS[month-1],mday,year,hour,minute,second);

  struct  timeval   unixtime;
  struct  timezone  unixtzone;
  time_t    ttime;
  tm        tmtime;

  memset(&tmtime,0,sizeof(tmtime));
  ttime=0;
  unixtime.tv_sec=0;
  unixtime.tv_usec=0;

  //gettimeofday(&unixtime,&unixtzone);
  //ttime=time(NULL);
  tmtime=*gmtime(&ttime);

  printf("TIME->CTIME = %s\n",ctime(&ttime));
  printf("GTOD->CTIE = %s\n",ctime(&(unixtime.tv_sec)));
  printf("TIME->GMT->ASCTIME = %s\n",asctime(&tmtime));
}
***************************************************************/



//
// Construct with current clock time
//
VeTime::VeTime( void )
{
    update();
}

//
// Copy constructor
//
VeTime::VeTime( const VeTime& other )
{
    day_ = other.day_;
    msec_ = other.msec_;
}

//
// Set to a time_t (1970-2038)
//
VeTime::VeTime( const time_t other )
{
    day_ = 719528;  // days from year 0 to Jan1, 1970
    // Add seconds from time_t
    addSeconds( int(other) );
    msec_ = 0;
}

VeTime::~VeTime()
{
}


time_t VeTime::getTime( void ) const
{
    int32 temp = day_ - 719528;  // days from year 0 to Jan1, 1970
    time_t retval = temp * 60 * 60 * 24;
    retval += ( msec_ / 1000 );

    return( retval );
}


//
// Add some number of seconds to the time (seconds can be negative)
//
void VeTime::addSeconds( int32 seconds )
{
    // Add to day counter first
    day_ += ( seconds / 86400 );
    msec_ += ( seconds % 86400 ) * 1000;

    // Now normalize in case msec is > 1 days worth
    normalize();
}



//
// Add some number of milliseconds to the time (seconds can be negative)
//
void VeTime::addMilliseconds( int32 milliseconds )
{
    // Add to day counter first
    day_ += ( milliseconds / 86400000 );
    msec_ += ( milliseconds % 86400000 );

    // Now normalize in case msec is > 1 days worth
    normalize();
}



//
// If msec is > 1 days worth, adjust the day count
//   & decrement the msec counter until OK.
//
void VeTime::normalize( void )
{
    day_ += ( msec_ / 86400000 );
    msec_ %= 86400000;

    while ( msec_ < 0 )
    {
        day_--;
        msec_ += 86400000;
    }
}

//
// Update time to hold the current clock time
// (breaks in 2038 :-)
//
void VeTime::update( void )
{
    day_ = 719528;  // day_s from year 0 to Jan1, 1970
    msec_ = 0;

#ifdef _WIN32
    struct _timeb wintime;
    _ftime( &wintime );
    addSeconds( int(wintime.time) );
    msec_ += wintime.millitm;
#endif
#ifndef _WIN32
    struct timeval unixtime;
    struct timezone unixtzone;
    gettimeofday( &unixtime, &unixtzone );
    addSeconds( unixtime.tv_sec );
    msec_ += ( unixtime.tv_usec / 1000 );
#endif

    // Now normalize in case msec is > 1 days worth
    normalize();
}


//
// Portable sleep function (like Sleep() or sleep() in msec)
//
void VeTime::sleep( int msec )
{
    timeval timeout;
    timeout.tv_sec = msec / 1000;
    timeout.tv_usec = ( msec % 1000 ) * 1000;
    select( 0, NULL, NULL, NULL, &timeout );
}


/**************************************

 REPLACE THIS CRAP

// Parses a date string that's in modified RFC 1123 format
// Can have a +minutes after the normal time
// eg: Thu, 20 Jun 1996 17:33:49 +100
// Returns true if successfully parsed, false otherwise
bool VeTime::ParseDate(char *in)
{
  int i;
  uint32 minOffset;
  struct tm t;
  char *ptr=in;
  while ((!isgraph(*ptr))&&(*ptr!=0)) ptr++;  // skip to start of string
  if (*ptr==0) return(false);
  t.tm_wday_=-1;
  for (i=0; i<7; i++)  // parse day_ of week
    if (strncmp(ptr,DAYS[i],strlen(DAYS[i]))==0)
      t.tm_wday_=i;
  if (t.tm_wday_==-1)
    return(false);
  while ((!isdigit(*ptr))&&(*ptr!=0)) ptr++;  // skip to day_ of month
  if (*ptr==0) return(false);
  t.tm_mday_=atoi(ptr);
  while ((!isalpha(*ptr))&&(*ptr!=0)) ptr++;  // skip to month
  if (*ptr==0) return(false);
  t.tm_mon=-1;
  for (i=0; i<12; i++)  // match month
    if (strncmp(ptr,MONTHS[i],strlen(MONTHS[i]))==0) t.tm_mon=i;
  if (t.tm_mon==-1) return(false);
  while ((!isdigit(*ptr))&&(*ptr!=0)) ptr++;
  if (*ptr==0) return(false);
  t.tm_year=atoi(ptr);
  if (t.tm_year<70)  // if they specify a 2 digit year, we'll be nice
    t.tm_year+=2000;
  else if (t.tm_year<100)
    t.tm_year+=1900;
  if (t.tm_year>2200)  // I doubt my code will be around for another 203 years
    return(false);
  while ((isdigit(*ptr))&&(*ptr!=0)) ptr++;  // skip to end of year
  if (*ptr==0) return(false);

  while ((!isgraph(*ptr))&&(*ptr!=0)) ptr++;  // skip to start of time
  if (*ptr==0) return(false);

  t.tm_hour=atoi(ptr);
  while ((*ptr!=':')&&(*ptr!=0)) ptr++;
  ptr++; // skip past colon
  if (*ptr==0) return(false);
  t.tm_min=atoi(ptr);
  while ((*ptr!=':')&&(*ptr!=0)) ptr++;
  ptr++; // skip past colon
  if (*ptr==0) return(false);
  t.tm_sec=atoi(ptr);
  t.tm_year%=100;   // 1996 is stored as 96, not 1996
  t.tm_isdst=-1;    // day_light savings info isn't available

  sec=(uint32)(mktime(&t));
  if ((int32)sec==-1)
    return(false);


  // The next part of the time is OPTIONAL (+minutes)

  // first skip past the seconds 
  while ((isdigit(*ptr))&&(*ptr!=0)) ptr++;
  if (*ptr==0) return(true);

  // skip past any spaces 
  while ((isspace(*ptr))&&(*ptr!=0)) ptr++;
  if (*ptr!='+')
  {
    //printf("\nNOPE ptr was '%s'\n",ptr);
    return(true);
  }
  ptr++;
  if (*ptr==0)
  {
    //printf("\nPTR WAS 0\n");
    return(true);
  }
 
  minOffset=atol(ptr);
  //printf("\n\nAdding %d minutes!\n\n",minOffset);
  sec+=minOffset*60;  // add the minutes as seconds
  return(true);
}


// This takes the standard Microsoft time formatting string
// make sure the out string is big enough
// An example format would be "mm/dd/yy hh:mm:ss"
bool VeTime::FormatTime(char *out, char *format)
{
  int lastWasH=0;
  out[0]=0;
  char *ptr=format;

  if (*ptr=='"') ptr++;  // skip past open quote if exists

  while (*ptr!=0)
  {
    if (lastWasH>0)
      lastWasH--;

    if (isspace(*ptr))
    {
      if (lastWasH==1) lastWasH=2;
      sprintf(out+strlen(out),"%c",*ptr);
      ptr+=1;
    } 
    else if (strncmp(ptr,"\"",1)==0)
    {
      break;
    }
    else if (strncmp(ptr,":",1)==0)
    {
      if (lastWasH==1) lastWasH=2;
      sprintf(out+strlen(out),":");
      ptr+=1;
    }
    else if (strncmp(ptr,"/",1)==0)
    {
      sprintf(out+strlen(out),"/");
      ptr+=1;
    }
    else if (strncmp(ptr,"c",1)==0)
    {
      sprintf(out+strlen(out),"%ld/%ld/%02ld %ld:%02ld:%02ld",GetMonth(),
        GetMDay(),GetYear()%100,GetHour(),GetMinute(),GetSecond());
      ptr+=1;
    }
    else if (strncmp(ptr,"dddddd",6)==0)
    {
      sprintf(out+strlen(out),"%s %02ld, %ld",FULLMONTHS[GetMonth()-1],
        GetMDay(),GetYear());
      ptr+=6;
    }
    else if (strncmp(ptr,"ddddd",5)==0)
    {
      sprintf(out+strlen(out),"%ld/%ld/%02ld",GetMonth(),GetMDay(),
        GetYear()%100);
      ptr+=5;
    }
    else if (strncmp(ptr,"dddd",4)==0)
    {
      sprintf(out+strlen(out),"%s",FULLDAYS[GetWDay()-1]);
      ptr+=4;
    }
    else if (strncmp(ptr,"ddd",3)==0)
    {
      sprintf(out+strlen(out),"%s",DAYS[GetWDay()-1]);
      ptr+=3;
    }
    else if (strncmp(ptr,"dd",2)==0)
    {
      sprintf(out+strlen(out),"%02ld",GetMDay());
      ptr+=2;
    }
    else if (strncmp(ptr,"d",1)==0)
    {
      sprintf(out+strlen(out),"%ld",GetMDay());
      ptr+=1;
    }
    else if (strncmp(ptr,"ww",2)==0)
    {
      sprintf(out+strlen(out),"%02ld",GetYWeek());
      ptr+=2;
    }
    else if (strncmp(ptr,"w",1)==0)
    {
      sprintf(out+strlen(out),"%ld",GetWDay());
      ptr+=1;
    }
    else if (strncmp(ptr,"mmmm",4)==0)
    {
      sprintf(out+strlen(out),"%s",FULLMONTHS[GetMonth()-1]);
      ptr+=4;
    }
    else if (strncmp(ptr,"mmm",3)==0)
    {
      sprintf(out+strlen(out),"%s",MONTHS[GetMonth()-1]);
      ptr+=3;
    }
    else if (strncmp(ptr,"mm",2)==0)
    {
      if (lastWasH==1)
        sprintf(out+strlen(out),"%02ld",GetMinute());
      else
        sprintf(out+strlen(out),"%02ld",GetMonth());
      ptr+=2;
    }
    else if (strncmp(ptr,"m",1)==0)
    {
      if (lastWasH==1)
        sprintf(out+strlen(out),"%ld",GetMinute());
      else
        sprintf(out+strlen(out),"%ld",GetMonth());
      ptr+=1;
    }
    else if (strncmp(ptr,"q",1)==0)
    {
      sprintf(out+strlen(out),"%ld",((GetMonth()-1)/4)+1);  // GetQuarter
      ptr+=1;
    } 
    else if (strncmp(ptr,"yyyy",4)==0)
    {
      sprintf(out+strlen(out),"%ld",GetYear());
      ptr+=4;
    }
    else if (strncmp(ptr,"yy",2)==0)
    {
      sprintf(out+strlen(out),"%02ld",GetYear()%100);
      ptr+=2;
    } 
    else if (strncmp(ptr,"y",1)==0)
    {
      sprintf(out+strlen(out),"%ld",GetYDay());
      ptr+=1;
    }
    else if (strncmp(ptr,"hh",2)==0)
    {
      sprintf(out+strlen(out),"%02ld",GetHour());
      lastWasH=2;  // needs to be 1 after top of loop decs it
      ptr+=2;
    }
    else if (strncmp(ptr,"h",1)==0)
    {
      sprintf(out+strlen(out),"%ld",GetHour());
      lastWasH=2;  // needs to be 1 after top of loop decs it
      ptr+=1;
    }
    else if (strncmp(ptr,"nn",2)==0)
    {
      sprintf(out+strlen(out),"%02ld",GetMinute());
      ptr+=2;
    }
    else if (strncmp(ptr,"n",1)==0)
    {
      sprintf(out+strlen(out),"%ld",GetMinute());
      ptr+=1;
    }
    else if (strncmp(ptr,"ss",2)==0)
    {
      sprintf(out+strlen(out),"%02ld",GetSecond());
      ptr+=2;
    }
    else if (strncmp(ptr,"s",1)==0)
    {
      sprintf(out+strlen(out),"%ld",GetSecond());
      ptr+=1;
    }
    else if (strncmp(ptr,"ttttt",5)==0)
    {
      sprintf(out+strlen(out),"%ld:%02ld:%02ld",GetHour(),GetMinute(),
        GetSecond());
      ptr+=5;
    }
// todo
// AM/PM am/pm A/P a/p AMPM
   else  // an unknown char, move to next
     ptr++;
  }
  return(true);
}



// In addition to PrintTime & PrintDate there is the 'Print' function
//   which prints both in RFC 1123 format

void VeTime::PrintTime(FILE *out) const
{
  char string[80];
  PrintTime(string);
  fprintf(out,"%s",string);
}

void VeTime::PrintTime(char *out) const
{
  sprintf(out," %02lu:%02lu:%02lu",GetHour(),GetMinute(),GetSecond());
}

void VeTime::PrintDate(FILE *out) const
{
  char string[80];
  PrintDate(string);
  fprintf(out,"%s",string);
}

void VeTime::PrintDate(char *out) const
{
  sprintf(out,"%s, %lu %s %lu",DAYS[GetWDay()-1],GetMDay(),MONTHS[GetMonth()-1],
    GetYear());
}
********************************************/

// Get day_s since year 0
int32 VeTime::getDay( void ) const
{
    return( day_ );
}

// Get msecs since start of day
int32 VeTime::getMsec( void ) const
{
    return( msec_ );
}

// Set days since year 0
void VeTime::setDay( int32 newday )
{
    day_ = newday;
}

// Set msec since start of this day
void VeTime::setMsec( int32 newmsec )
{
    msec_ = newmsec;
}

// Set both
void VeTime::set( int32 newday, int32 newmsec )
{
    day_ = newday;
    msec_ = newmsec;
}


//
// Get a timeval ptr from a VeTime class
// May fail if timeval can't hold a year this big or small
//
bool VeTime::getTimeval( struct timeval& tv ) const
{
    // A timeval can only hold dates from 1970-2038
    if ( ( day_ < 719528 ) || ( day_ >= 719528 + 24855 ) )
        return( false );

    // Compute seconds since Jan 1, 1970
    uint32 seconds = day_ - 719528;
    seconds *= ( 60 * 60 * 24 );
    seconds += ( msec_ / 1000 );

    tv.tv_sec = seconds;
    tv.tv_usec = ( msec_ % 1000 ) * 1000;
    return( true );
}

//
// Set the time
//
bool VeTime::setTime( int month, int mday, int year, int hour, int minute, int second )
{
    day_ = Get_Day( month, mday, year );
    msec_ = ( hour * 1000 * 60 * 60 ) + ( minute * 1000 * 60 ) + ( second * 1000 );
    return( true );
}


//
// Get all the components of the time in the usual normalized format.
//
// Most of the uglyness is in Get_Date_From_Day() 
//
bool VeTime::getTime( int& month, int& mday, int& year, int& hour, int& minute, int& second ) const
{
    int i;
    int dayofyear;
    if ( Get_Date_From_Day( day_, year, dayofyear ) == false )
        return( false );

    static int DaysAtMonth[2][12] =
    {
        {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},   // normal
        {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}    // leap year

    };

    month = 0;

    bool isleap = IS_LEAP( year );
    for ( i = 0; i < 12; i++ )
    {
        if ( DaysAtMonth[isleap][i] > dayofyear )
            break;
        month = i;
    }
    month++;  // 1 based

    mday = dayofyear - DaysAtMonth[isleap][month - 1];

    // Whew! Now all we have to do is figure out H/M/S from the msec!
    hour = ( msec_ / 3600000 ) % 24;  // 1000*60*60
    minute = ( msec_ / 60000 ) % 60;  // 1000*60 
    second = ( msec_ / 1000 ) % 60;  // 1000

    return( true );
}

//
// These are for getting components of the time in the
//   standard ranges.  Like Day 1-31, Second 0-59, etc...
//
int VeTime::getSecond( void ) const
{
    int month, mday, year, hour, minute, second;
    getTime( month, mday, year, hour, minute, second );
    return( second );
}
int VeTime::getMinute( void ) const
{
    int month, mday, year, hour, minute, second;
    getTime( month, mday, year, hour, minute, second );
    return( minute );
}
int VeTime::getHour( void ) const
{
    int month, mday, year, hour, minute, second;
    getTime( month, mday, year, hour, minute, second );
    return( hour );
}
int VeTime::getMDay( void ) const
{
    int month, mday, year, hour, minute, second;
    getTime( month, mday, year, hour, minute, second );
    return( mday );
}

int VeTime::getMonth( void ) const
{
    int month, mday, year, hour, minute, second;
    getTime( month, mday, year, hour, minute, second );
    return( month );
}

// based at year 0 (real 0, not 1970)
int VeTime::getYear( void ) const
{
    int month, mday, year, hour, minute, second;
    getTime( month, mday, year, hour, minute, second );
    return( year );
}


//
// Set the seconds value (0-59)
//
bool VeTime::setSecond( int32 sec )
{
    int32 second = ( msec_ / 1000 ) % 60;
    msec_ -= ( second * 1000 );
    msec_ += ( sec * 1000 );
    return( true );
}

//
// Set the minutes value (0-59)
//
bool VeTime::setMinute( int32 min )
{
    int32 minute = ( msec_ / 60000 ) % 60;  // 1000*60 
    msec_ -= ( minute * 60000 );
    msec_ += ( min * 60000 );
    return( true );
}

//
// Set the minutes value (0-23)
//
bool VeTime::setHour( int32 hour )
{
    hour = ( msec_ / 3600000 ) % 24;  // 1000*60*60
    msec_ -= ( hour * 3600000 );
    msec_ += ( hour * 3600000 );
    return( true );
}

//
// Set the year value
// Results are undefined if you're moving from Feb 29, to a non leap year
//
bool VeTime::setYear( int32 _year )
{
    // extract the date
    int month, mday, year, hour, min, sec;
    getTime( month, mday, year, hour, min, sec );

    // modify & rebuild
    year = _year;
    day_ = Get_Day( month, mday, year );
    return( true );
}

//
// Modify the month
//
bool VeTime::setMonth( int32 _month )
{
    // extract the date
    int month, mday, year, hour, min, sec;
    getTime( month, mday, year, hour, min, sec );

    // modify & rebuild
    month = _month;
    day_ = Get_Day( month, mday, year );
    return( true );
}


//
// Modify the day of the month 
//
bool VeTime::setMDay( int32 _mday )
{
    // extract the date
    int month, mday, year, hour, min, sec;
    getTime( month, mday, year, hour, min, sec );

    // modify & rebuild
    mday = _mday;
    day_ = Get_Day( month, mday, year );
    return( true );
}


//
// Compare two times.  The time better be normalized
//   which it would be unless you've put bad stuff in it!
//
// 1 = *this > other
//-1 = *this < other
// 0 = *this == other
int VeTime::compare( const VeTime& other ) const
{
    if ( ( day_ == other.day_ ) && ( msec_ == other.msec_ ) )
        return( 0 );        // equal

    else if ( day_ > other.day_ )
        return( 1 );
    else if ( day_ < other.day_ )
        return( -1 );
    else if ( msec_ > other.msec_ )
        return( 1 );
    else
        return( -1 );
}


bool VeTime::operator ==( const VeTime& other ) const
{
    bool retval = compare( other ) ? true : false;
    if ( retval == 0 )
        return( true );
    else
        return( false );
}

bool VeTime::operator !=( const VeTime& other ) const
{
    bool retval = compare( other ) ? true : false;
    if ( retval == 0 )
        return( false );
    else
        return( true );
}

bool VeTime::operator <( const VeTime& other ) const
{
    int retval = compare( other );
    if ( retval == -1 )
        return( true );
    else
        return( false );
}

bool VeTime::operator >( const VeTime& other ) const
{
    int retval = compare( other );
    if ( retval == 1 )
        return( true );
    else
        return( false );
}

bool VeTime::operator <=( const VeTime& other ) const
{
    int retval = compare( other );
    if ( ( retval == -1 ) || ( retval == 0 ) )
        return( true );
    else
        return( false );
}

bool VeTime::operator >=( const VeTime& other ) const
{
    int retval = compare( other );
    if ( ( retval == 1 ) || ( retval == 0 ) )
        return( true );
    else
        return( false );
}


VeTime& VeTime::operator +=( const VeTime& other )
{
    day_ += other.day_;
    msec_ += other.msec_;
    normalize();
    return *this;
}

VeTime& VeTime::operator -=( const VeTime& other )
{
    day_ -= other.day_;
    msec_ -= other.msec_;
    normalize();
    return *this;
}

VeTime VeTime::operator -( VeTime& other )
{
    VeTime temp( *this );
    temp -= other;
    return( temp );
}

VeTime VeTime::operator +( VeTime& other )
{
    VeTime temp( *this );
    temp += other;
    return( temp );
}


VeTime& VeTime::operator =( const VeTime& other )
{
    day_ = other.day_;
    msec_ = other.msec_;
    return *this;
}


VeTime& VeTime::operator +=( const time_t other )
{
    addSeconds( static_cast<int32>(other) );
    return *this;
}


VeTime& VeTime::operator -=( const time_t other )
{
    addSeconds( -( ( int32 ) other ) );
    return *this;
}


VeTime VeTime::operator -( time_t other )
{
    VeTime temp( *this );
    temp -= other;
    return( temp );
}


VeTime VeTime::operator +( time_t other )
{
    VeTime temp( *this );
    temp += other;
    return( temp );
}

VeTime& VeTime::operator =( const time_t other )
{
    msec_ = 0;
    day_ = 719528;  // Jan 1, 1970
    addSeconds( static_cast<int32>(other) );
    return *this;
}
