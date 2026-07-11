// ======================================================================
// Unicode.h
// copyright (c) 2001 Sony Online Entertainment
//
// jwatson
//
// Basic Unicode primitives and string handling/manipulating functions
// ======================================================================

#ifndef INCLUDED_Unicode_H
#define INCLUDED_Unicode_H

#if WIN32
#pragma warning (disable:4710)
#pragma warning (disable:4786)
// stl warning func not inlined
#pragma warning (disable:4514)
#endif

#include <string>

//-----------------------------------------------------------------

namespace Unicode
{
	using unicode_char_t = char16_t;   // fixed 16-bit
	using String = std::u16string;

	/**
	* NarrowString is a ascii string.
	*/

	typedef std::string                        NarrowString;

	// Unicode whitespace/endline tables must also use wchar_t
	inline constexpr unicode_char_t whitespace[] = {
		u' ', u'\n', u'\r', u'\t', 0x3000, 0
	};

	inline constexpr unicode_char_t endlines[] = { u'\r', u'\n', 0 };

	inline constexpr char ascii_whitespace[] = { ' ', '\n', '\r', '\t', 0 };
	inline constexpr char ascii_endlines[] = { '\r', '\n', 0 };

	inline const String emptyString;
}

//-----------------------------------------------------------------

#endif

