//======================================================================
//
// CuiStringIdsGroup.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsGroup_H
#define INCLUDED_CuiStringIdsGroup_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace CuiStringIdsGroup
{
	MAKE_STRING_ID(group,      other_left_prose);
	MAKE_STRING_ID(group,      other_joined_prose);
	MAKE_STRING_ID(group,      out_of_range_suffix);
};

#undef MAKE_STRING_ID
//======================================================================

#endif
