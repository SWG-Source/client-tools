//======================================================================
//
// CuiUtils.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiUtils.h"

#include "UIUtils.h"
#include "clientUserInterface/CuiStringIds.h"
#include "sharedMath/PackedArgb.h"
#include "sharedMath/Vector.h"
#include "sharedMath/Vector2d.h"
#include "sharedMath/VectorArgb.h"
#include <sys/timeb.h>

//======================================================================

namespace CuiUtils
{
	const size_t secsPerMinute = 60;
	const size_t secsPerHour   = secsPerMinute * 60;
	const size_t secsPerDay    = secsPerHour   * 24;

	//----------------------------------------------------------------------

	bool	ParseVector   ( const UIString & str, Vector & v   )
	{
		size_t commas [3];
		size_t pos = 0;
		
		for (int i = 0; i < 2; ++i)
		{
			commas [i] = str.find (',', pos);
			
			if (commas [i] == str.npos)
				return false;
			
			pos = commas [i] + 1;
		}
		
		UINarrowString s (UIUnicode::wideToNarrow (str));
		
		v.x   = static_cast<float>(atof( s.substr (0,             commas[0]).c_str() ));
		v.y   = static_cast<float>(atof( s.substr (commas[0] + 1, commas[1]).c_str() ));
		v.z   = static_cast<float>(atof( s.substr (commas[1] + 1, commas[2]).c_str() ));
		return true;
	}

	
	//----------------------------------------------------------------------

	bool	ParseVector2d (const UIString & str, Vector2d & v)
	{
		size_t commas [2];
		size_t pos = 0;
		
		for (int i = 0; i < 1; ++i)
		{
			commas [i] = str.find (',', pos);
			
			if (commas [i] == str.npos)
				return false;
			
			pos = commas [i] + 1;
		}
		
		const UINarrowString & s = UIUnicode::wideToNarrow (str);
		
		v.x   = static_cast<float>(atof( s.substr (0,             commas[0]).c_str() ));
		v.y   = static_cast<float>(atof( s.substr (commas[0] + 1, commas[1]).c_str() ));
		return true;
	}


	//----------------------------------------------------------------------

	bool	ParseVectorArgb   ( const UIString & str, VectorArgb &  v  )
	{
		UIColor color;
		if (UIUtils::ParseColor (str, color))
		{
			v.a = static_cast<float>(color.a) / 255.0f;
			v.r = static_cast<float>(color.r) / 255.0f;
			v.g = static_cast<float>(color.g) / 255.0f;
			v.b = static_cast<float>(color.b) / 255.0f;
			return true;
		}
		return false;
	}
	
	//----------------------------------------------------------------------

	bool  FormatVector2d   (Unicode::String & str, const Vector2d & v)
	{
		char Buffer[128];		
		snprintf(Buffer, 128, "%g,%g", v.x, v.y);
		str = UIUnicode::narrowToWide (Buffer);
		return true;
	}

	//----------------------------------------------------------------------

	bool	FormatVector  (UIString & str, const Vector & v)
	{
		char Buffer[128];		
		snprintf(Buffer, 128, "%g,%g,%g", v.x, v.y, v.z);
		str = UIUnicode::narrowToWide (Buffer);
		return true;
	}

	//----------------------------------------------------------------------

	bool	FormatVectorArgb  ( UIString & str, const VectorArgb &  v  )
	{
		const UIColor color (static_cast<uint8>(v.r * 255.0f),
			static_cast<uint8>(v.g * 255.0f),
			static_cast<uint8>(v.b * 255.0f),
			static_cast<uint8>(v.a * 255.0f));

		return UIUtils::FormatColor (str, color);

	}

	//----------------------------------------------------------------------

	const UIColor convertColor     (const PackedArgb & pargb)
	{		
		return UIColor (pargb.getR (), pargb.getG (), pargb.getB (), pargb.getA ());
	}

	//----------------------------------------------------------------------

	const PackedArgb  convertColor     (const UIColor & color)
	{
		return PackedArgb (color.a, color.r, color.g, color.b);
	}

	//----------------------------------------------------------------------

	const VectorArgb  convertToVectorArgb (const UIColor & color)
	{
		return VectorArgb (convertColor (color));
	}

	const UIColor           convertColor     (const VectorArgb & vargb)
	{
		return UIColor (static_cast<uint8>(255 * vargb.r),
			static_cast<uint8>(255 * vargb.g),
			static_cast<uint8>(255 * vargb.b),
			static_cast<uint8>(255 * vargb.a));
	}

	////----------------------------------------------------------------------
	//
	//void GetTime (time_t systemSeconds, int &days, int &hour, int &minute, int &second)
	//{
	//	static const int secondsPerMinute = 60;
	//	static const int secondsPerHour   = secondsPerMinute * 60;
	//	static const int secondsPerDay    = secondsPerHour   * 24;
	//
	//	int seconds = static_cast<int>(secs);
	//
	//	// Get the hour
	//
	//	hour = seconds / secondsPerHour;
	//
	//	if (hour > 0)
	//	{
	//		seconds -= hour * secondsPerHour;
	//	}
	//
	//	// Get the minute
	//
	//	minutes = seconds / secondsPerMinute;
	//
	//	if (minuntes)
	//	{
	//		seconds -= minutes * secondsPerMinute;
	//	}
	//
	//	// Get the second
	//
	//	second = seconds;
	//}

	//----------------------------------------------------------------------

	bool                    FormatDate       (Unicode::String & str, time_t secs)
	{
		std::string rawTime (ctime (&secs));

		//-- remove the endline

		if (!rawTime.empty ())
			rawTime.erase (rawTime.size () - 1);

		str = Unicode::narrowToWide (rawTime);
		return true;
	}

	bool CuiUtils::FormatTimeDuration (Unicode::String & str, time_t secs, bool showDays, bool showHours, bool showMins, bool showSecs, bool shortHand)
	{
		time_t days  = 0;
		time_t hours = 0;
		time_t mins  = 0;

		if (showDays)
		{
			days = secs / secsPerDay;
			if (days)
				secs -= days * secsPerDay;
		}

		if (showHours)
		{
			hours = secs / secsPerHour;
			if (hours)
				secs -= hours * secsPerHour;
		}

		if (showMins)
		{
			mins = secs / secsPerMinute;
			if (mins)
				secs -= mins * secsPerMinute;
		}

		Unicode::String tmpStr;

		if (showDays && days)
		{
			UIUtils::FormatInteger (tmpStr, int(days));
			str.append (tmpStr);

			if (shortHand)
			{
				str.push_back (CuiStringIds::token_days.localize ()[0]);
			}
			else
			{
				str.push_back (' ');
				str.append (CuiStringIds::token_days.localize ());
			}
		}

		if (showHours && hours)
		{
			if (!str.empty ())
			{
				if (!shortHand)
					str.push_back (',');
				str.push_back (' ');
			}

			UIUtils::FormatInteger (tmpStr, int(hours));
			str.append (tmpStr);

			if (shortHand)
			{
				str.push_back (CuiStringIds::token_hours.localize ()[0]);
			}
			else
			{
				str.push_back (' ');
				str.append (CuiStringIds::token_hours.localize ());
			}
		}

		if (showMins && mins)
		{
			if (!str.empty ())
			{
				if (!shortHand)
					str.push_back (',');
				str.push_back (' ');
			}

			UIUtils::FormatInteger (tmpStr, int(mins));
			str.append (tmpStr);
			if (shortHand)

			{
				str.push_back (CuiStringIds::token_minutes_abbrev.localize ()[0]);
			}
			else
			{
				str.push_back (' ');
				str.append (CuiStringIds::token_minutes_abbrev.localize ());
			}
		}

		if ((showSecs || str.empty ()) && secs)
		{
			if (!str.empty ())
			{
				if (!shortHand)
					str.push_back (',');
				str.push_back (' ');
			}

			UIUtils::FormatInteger (tmpStr, int(secs));
			str.append (tmpStr);

			if (shortHand)
			{
				str.push_back (CuiStringIds::token_seconds_abbrev.localize ()[0]);
			}
			else
			{
				str.push_back (' ');
				str.append (CuiStringIds::token_seconds_abbrev.localize ());
			}
		}

		return true;
	}

	// Returns the time in seconds since UTC 1/1/70

	time_t GetSystemSeconds()
	{
		time_t ltime;
		time(&ltime);

		return ltime;
	}

	// Returns the time zone difference from UTC in seconds

	time_t GetTimeZoneDifferenceFromUTC()
	{
		struct _timeb tstruct;

		_ftime(&tstruct);

		return tstruct.timezone * secsPerMinute;
	}
}

//======================================================================
