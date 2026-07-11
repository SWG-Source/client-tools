// ======================================================================
//
// UIString.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef __UISTRING_H__
#define __UISTRING_H__

#include "Unicode.h"

#include <windows.h>
#include <string>
#include <cassert>

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

    static_assert(sizeof(char16_t) == 2, "char16_t must be 16-bit");
    static_assert(sizeof(wchar_t) == 2, "wchar_t must be 16-bit on Windows");

    // Case-insensitive compare for UTF-16 code units (Windows ordinal rules)
    inline int icmp(const UIString& s1, const UIString& s2)
    {
        const wchar_t* w1 = reinterpret_cast<const wchar_t*>(s1.c_str());
        const wchar_t* w2 = reinterpret_cast<const wchar_t*>(s2.c_str());

        // -1 => null-terminated
        const int r = ::CompareStringOrdinal(w1, -1, w2, -1, TRUE);
        // Return like strcmp: <0, 0, >0
        return (r == CSTR_LESS_THAN) ? -1 : (r == CSTR_GREATER_THAN) ? 1 : 0;
    }

    inline int nicmp(const UIString& s1, const UIString& s2, size_t len)
    {
        const wchar_t* w1 = reinterpret_cast<const wchar_t*>(s1.c_str());
        const wchar_t* w2 = reinterpret_cast<const wchar_t*>(s2.c_str());

        // CompareStringOrdinal expects int lengths in code units
        const int n = (len > static_cast<size_t>(INT_MAX)) ? INT_MAX : static_cast<int>(len);

        const int r = ::CompareStringOrdinal(w1, n, w2, n, TRUE);
        return (r == CSTR_LESS_THAN) ? -1 : (r == CSTR_GREATER_THAN) ? 1 : 0;
    }

	UIString narrowToWide (const UINarrowString & nstr);

	UINarrowString wideToNarrow (const UIString & str);
}

//-----------------------------------------------------------------

#ifndef UI_UNICODE_T
#define UI_UNICODE_T(s) (u##s)
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
