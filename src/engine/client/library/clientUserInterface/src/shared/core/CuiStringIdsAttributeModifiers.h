
//======================================================================
//
// CuiStringIdsAttributeModifiers.h
// Copyright Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsAttributeModifiers_H
#define INCLUDED_CuiStringIdsAttributeModifiers_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------
namespace CuiStringIdsAttributeModifiers
{
	MAKE_STRING_ID(ui_attrib_mod, name);
	MAKE_STRING_ID(ui_attrib_mod, description);
	MAKE_STRING_ID(ui_attrib_mod, timeleft);
	MAKE_STRING_ID(ui_attrib_mod, seconds);
};

#undef MAKE_STRING_ID

//======================================================================

#endif // INCLUDED_CuiStringIdsAttributeModifiers_H
