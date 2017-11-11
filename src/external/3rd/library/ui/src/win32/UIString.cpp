// ======================================================================
//
// UIString.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "_precompile.h"
#include "UIString.h"
#include "UnicodeUtils.h"

//-----------------------------------------------------------------
//-- wrappers for future platform independence
//-----------------------------------------------------------------

namespace UIUnicode
{

	UIString narrowToWide (const UINarrowString & nstr)
	{
		return Unicode::narrowToWide (nstr);
	}

	UINarrowString wideToNarrow (const UIString & str)
	{
		return Unicode::wideToNarrow (str);
	}
}

//-----------------------------------------------------------------
