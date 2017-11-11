// \addtodoc

#ifndef INC_NXNTIME_H
#define INC_NXNTIME_H

/*!	\file		NxNTime.h
 *	
 *	\brief		This file contains the definition for the class CNxNTime and its associated types.
 *	
 *	\author		Axel Pfeuffer
 *	
 *	\version	1.00
 *	
 *	\date		2000
 *	
 *	\mod
 *	\endmod
 */

/* \class		CNxNTime NxNTime.h
 *
 *  \brief		This class contains method to handle time values in different formats and converting between them.
 *				
 *  \author		Axel Pfeuffer, (c) 1999-2001 by NxN Software AG
 *
 *  \version	1.0
 *
 *  \date		2001
 *
 * 	\mod
 *		[ap]-07-May-2001 file created.
 *	\endmod
 */

//---------------------------------------------------------------------------
//	include needed for _timeb structure
//---------------------------------------------------------------------------
#include <sys/timeb.h>

//---------------------------------------------------------------------------
//	hidden structure for data storage and a type def for 64 bit integers
//---------------------------------------------------------------------------
typedef struct _TNxNTimeData*	TNxNTimeData;
typedef __int64                 TNxNTime;

//---------------------------------------------------------------------------
//	enumerated time format for the conversion into a displayable string
//---------------------------------------------------------------------------
/*!	\enum eNxNTimeFormat
 *		These enumerated format flags are for use with the CNxNTime::ToDisplayString() objects. 
 */
/*!	\var eNxNTimeFormat NXN_TIME_FORMAT_DEFAULT	
 *		Converts the time stamp to a string of the same format as NXN_TIME_FORMAT_DETAIL_LOCAL does.
 */
/*!	\var eNxNTimeFormat NXN_TIME_FORMAT_DETAIL_UTC	
 *		Converts the time stamp to a detailed string using the universal time format, i.e. "Saturday, September 23, 2000, 07:45:08 PM".
 */
/*!	\var eNxNTimeFormat NXN_TIME_FORMAT_DETAIL_LOCAL	
 *		Converts the time stamp to a detailed string using the local time format, i.e. "Saturday, September 23, 2000, 07:45:08 PM"
 */
/*!	\var eNxNTimeFormat NXN_TIME_FORMAT_SIMPLE_UTC	
 *		Converts the time stamp to a simple string using the universal time format, i.e. "09/23/00 18:06:47"
 */
/*!	\var eNxNTimeFormat NXN_TIME_FORMAT_SIMPLE_LOCAL	
 *		Converts the time stamp to a simple string using the local time format, i.e. "09/23/00 18:06:47"
 */
/*!	\var eNxNTimeFormat NXN_TIME_FORMAT_DATE_UTC	
 *		Converts the time stamp to a date string using the universal time format, i.e. "09/23/00"
 */
/*!	\var eNxNTimeFormat NXN_TIME_FORMAT_DATE_LOCAL	
 *		Converts the time stamp to a date string using the local time format, i.e. "09/23/00"
 */
/*!	\var eNxNTimeFormat NXN_TIME_FORMAT_TIME_UTC	
 *		Converts the time stamp to a time string using the universal time format, i.e. "07:45:08 PM"
 */
/*!	\var eNxNTimeFormat NXN_TIME_FORMAT_TIME_LOCAL	
 *		Converts the time stamp to a time string using the local time format, i.e. "07:45:08 PM"
 */
enum eNxNTimeFormat
{	
	NXN_TIME_FORMAT_DEFAULT		    = 0,	// same as NXN_TIME64_FORMAT_DETAIL_LOCAL
	NXN_TIME_FORMAT_DETAIL_UTC	    = 1,	// Universal - "Saturday, September 23, 2000, 07:45:08 PM"
	NXN_TIME_FORMAT_DETAIL_LOCAL	= 2,	// Local - "Saturday, September 23, 2000, 07:45:08 PM"
	NXN_TIME_FORMAT_SIMPLE_UTC	    = 3,	// Universal - "09/23/00 18:06:47"
	NXN_TIME_FORMAT_SIMPLE_LOCAL	= 4,	// Local - "09/23/00 18:06:47"
	NXN_TIME_FORMAT_DATE_UTC		= 5,	// Universal - "09/23/00"
	NXN_TIME_FORMAT_DATE_LOCAL	    = 6,	// Local - "09/23/00"
	NXN_TIME_FORMAT_TIME_UTC		= 7,	// Universal - "07:45:08 PM"
	NXN_TIME_FORMAT_TIME_LOCAL	    = 8,	// Local - "07:45:08 PM"
};

/*!	\var NXN_TIME_MILLISECOND 
 *		10000 100-nanosecond intervals
 */
/*!	\var NXN_TIME_SECOND 
 *		10000000 100-nanosecond intervals
 */
/*!	\var NXN_TIME_LEGACY_MAX 
 *		anything below is assumed a legacy value (before ca. 1604)
 */
/*!	\var NXN_TIME_BASEOFFSET 
 *		100-nanosecond intervals from 1601 to 1970
 */
/*!	\var NXN_TIME_DISKDELTA 
 *		mod. time resolution
 */
const TNxNTime NXN_TIME_MILLISECOND     =              10000;	// 10000 100-nanosecond intervals
const TNxNTime NXN_TIME_SECOND		    =           10000000;	// 10000000 100-nanosecond intervals
const TNxNTime NXN_TIME_LEGACY_MAX	    =   1000000000000000;	// anything below is assumed a legacy value (before ca. 1604)
const TNxNTime NXN_TIME_BASEOFFSET	    = 116444736000000000;	// 100-nanosecond intervals from 1601 to 1970
const TNxNTime NXN_TIME_DISKDELTA	    = 2 * NXN_TIME_SECOND;  // mod. time resolution

class NXNINTEGRATORSDK_API CNxNTime : public CNxNObject
{
	NXN_DECLARE_DYNCREATE(CNxNTime);

	public:
        //---------------------------------------------------------------------------
        //	construction/destruction
        //---------------------------------------------------------------------------
		CNxNTime();
		CNxNTime(TNxNTime tTime);
        CNxNTime(const CNxNTime& tTime);

        CNxNTime(const FILETIME& ftFileTime);
        CNxNTime(const CNxNString& sStorageString);

		virtual ~CNxNTime();

        //---------------------------------------------------------------------------
        //	assignment operator
        //---------------------------------------------------------------------------
        const CNxNTime& operator =(const CNxNTime& tTime) const;
        const CNxNTime& operator =(const TNxNTime& tTime) const;

        //---------------------------------------------------------------------------
        //	Information retrieval
        //---------------------------------------------------------------------------
        TNxNTime GetTime() const;

        static CNxNTime Current();
        static TNxNTime GetCurrentTime();
        TNxNTime GetCurrent() const;

        inline operator TNxNTime() const { return ToTNxNTime(); };
        operator TNxNTime&() const;
        operator const TNxNTime&() const;
      
        inline operator time_t() const { return ToTime_t(); };
  
        //---------------------------------------------------------------------------
        //	conversion methods
        //---------------------------------------------------------------------------
        CNxNString ToDisplayString(eNxNTimeFormat eFormat = NXN_TIME_FORMAT_DEFAULT) const;

		time_t	ToTime_t() const;
        TNxNTime ToTNxNTime() const;
        FILETIME ToFILETIME() const;

		CNxNString ToStorageString() const;
        void FromStorageString(const CNxNString& sStorageString) const;

		static CNxNString ToDisplayString(TNxNTime tTime, eNxNTimeFormat eFormat = NXN_TIME_FORMAT_DEFAULT);

		static TNxNTime	ToTNxNTime(FILETIME ftFileTime);					//	FILETIME -> __int64
		static TNxNTime	ToTNxNTime(time_t tTime, bool bAssumeLocal = true);	//	time_t -> __int64
		static TNxNTime	ToTNxNTime(_timeb tTime);					//	_timeb -> __int64

        static time_t	ToTime_t(TNxNTime tTime);					//	__int64 -> time_t
		static FILETIME	ToFILETIME(TNxNTime tTime);		            //	__int64 -> FILETIME
		static SYSTEMTIME ToSYSTEMTIME(TNxNTime tTime);	            //	__int64 -> SYSTEMTIME
		static CNxNString ToStorageString(TNxNTime tTime);	        //	__int64 -> "%I64d"

		static TNxNTime	TimeFromStorageString(const CNxNString& sStorageString);	//	"%I64d" -> __int64

        static TNxNTime UTCToLocal(TNxNTime tTime, bool bCheckDST = false);	//	__int64 -> __int64 (UTC -> Local)
		static TNxNTime LocalToUTC(TNxNTime tTime, bool bCheckDST = true);	//	__int64 -> __int64 (Local -> UTC)

        static CNxNString FormatUTC(LPCWSTR pFormat, TNxNTime tTime);

        //---------------------------------------------------------------------------
        //	object validity check
        //---------------------------------------------------------------------------
        bool IsValid() const;

        //---------------------------------------------------------------------------
        //	access to hidden data
        //---------------------------------------------------------------------------
        TNxNTimeData GetTimeData() const
        {
            return m_pTimeData;
        }

    private:
        TNxNTimeData    m_pTimeData;
};

#endif // INC_NXNTIME_H
