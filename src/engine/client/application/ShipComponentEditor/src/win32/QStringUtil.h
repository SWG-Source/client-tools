//======================================================================
//
// QStringUtil.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_QStringUtil_H
#define INCLUDED_QStringUtil_H

//======================================================================

#include "UnicodeUtils.h"

//----------------------------------------------------------------------

namespace QStringUtil
{
	inline Unicode::String toUnicode(QString const & qstring)
	{
		if (NULL != qstring.latin1())
			return Unicode::narrowToWide(qstring.latin1());
		else
			return Unicode::emptyString;
	}

	inline std::string toString(QString const & qstring)
	{
		if (NULL != qstring.latin1())
			return std::string(qstring.latin1());
		else
			return std::string();
	}

	//-- these are needed to prevent accidents arising from Qt's misuse of QString implicit conversion
	void toString(char const * const str);
	void toUnicode(char const * const str);
}

//======================================================================

#endif
