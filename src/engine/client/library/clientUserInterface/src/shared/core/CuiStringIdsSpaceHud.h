//======================================================================
//
// CuiStringIdsSpaceHud.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsSpaceHud_H
#define INCLUDED_CuiStringIdsSpaceHud_H

//======================================================================

#include "StringId.h"

//----------------------------------------------------------------------

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

namespace CuiStringIdsSpaceHud
{
	MAKE_STRING_ID(ui_spacehud, weapon_group);
	MAKE_STRING_ID(ui_spacehud, selectedgroupprose);

	MAKE_STRING_ID(ui_spacehud, component_enabled);
	MAKE_STRING_ID(ui_spacehud, component_disabled);
	MAKE_STRING_ID(ui_spacehud, component_lowpower);
	MAKE_STRING_ID(ui_spacehud, component_fullpower);
	MAKE_STRING_ID(ui_spacehud, component_active);
	MAKE_STRING_ID(ui_spacehud, component_inactive);
	MAKE_STRING_ID(ui_spacehud, component_demolished);
	MAKE_STRING_ID(ui_spacehud, component_repaired);
	MAKE_STRING_ID(ui_spacehud, component_needspower);
	MAKE_STRING_ID(ui_spacehud, component_haspower);
}

//======================================================================

#endif
