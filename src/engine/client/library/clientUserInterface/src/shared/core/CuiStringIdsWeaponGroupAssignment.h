//======================================================================
//
// CuiStringIdsWeaponGroupAssignment.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsWeaponGroupAssignment_H
#define INCLUDED_CuiStringIdsWeaponGroupAssignment_H

//======================================================================
#include "StringId.h"

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace CuiStringIdsWeaponGroupAssignment
{
	MAKE_STRING_ID(ui_weapongroupassignment, suffix_empty);
	MAKE_STRING_ID(ui_weapongroupassignment, suffix_missile);
	MAKE_STRING_ID(ui_weapongroupassignment, suffix_projectile);
	MAKE_STRING_ID(ui_weapongroupassignment, suffix_countermeasure);
}

//======================================================================

#endif
