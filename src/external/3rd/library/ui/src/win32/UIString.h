// ======================================================================
//
// UIString.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef __UISTRING_H__
#define __UISTRING_H__

#include "Unicode.h"

//-- narrow string
typedef Unicode::NarrowString                       UINarrowString;
//-- must use unsigned short explicitly because wchar_t is platform dependant
typedef Unicode::String                             UIString;

//-----------------------------------------------------------------
//-- wrappers for future platform independence
//-----------------------------------------------------------------

namespace UIUnicode
{
	typedef Unicode::unicode_char_t unicode_char_t;
	inline int icmp (const UIString & s1, const UIString & s2)
	{
		return _wcsicmp (s1.c_str (), s2.c_str ()); //lint !e64 //type mismatch
	}

	inline int nicmp (const UIString & s1, const UIString & s2, size_t len)
	{
		return _wcsnicmp (s1.c_str (), s2.c_str (), len); //lint !e64 //type mismatch
	}

	UIString narrowToWide (const UINarrowString & nstr);

	UINarrowString wideToNarrow (const UIString & str);
}

//-----------------------------------------------------------------

#ifndef UI_UNICODE_T
#define UI_UNICODE_T(s) (L##s)
#endif

//-----------------------------------------------------------------

class CompareNoCase
{
public:
	bool operator()( const char * const a, const char * const b ) const { return _stricmp( a, b ) < 0; };
};

//-----------------------------------------------------------------

class UIStringCompareNoCase
{
public:
	bool operator()( const UIString & a, const UIString & b ) const
	{ 
		return UIUnicode::icmp (a, b) < 0;
	};
};

//-----------------------------------------------------------------

#endif
