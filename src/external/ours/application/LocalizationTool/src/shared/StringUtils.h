// ======================================================================
//
// StringUtils.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_StringUtils_H
#define INCLUDED_StringUtils_H

#include "Unicode.h"
#include <qstring.h>

//-----------------------------------------------------------------

namespace StringUtils
{
	
	inline Unicode::String convertQString (const QString & qstr)
	{
		Unicode::String str;
		str.reserve (qstr.length ());
		
		for (size_t i = 0; i < qstr.length (); ++i)
		{
			str.append (1, qstr.at (i).unicode ());
		}
		
		return str;
	}

	//-----------------------------------------------------------------
	
	inline Unicode::NarrowString convertQStringNarrow (const QString & qstr)
	{
		Unicode::NarrowString str;
		str.reserve (qstr.length ());
		
		for (size_t i = 0; i < qstr.length (); ++i)
		{
			str.append (1, qstr.at (i).latin1 ());
		}
		
		return str;
	}

	//-----------------------------------------------------------------
	
	inline QString convertUnicode (const Unicode::String & str)
	{
		QString qstr;
		
		const Unicode::String::const_iterator end = str.end ();

		for (Unicode::String::const_iterator it = str.begin (); it != end; ++it)
		{
			qstr.append (QChar (*it));
		}
		
		return qstr;
	}
}

// ======================================================================

#endif
