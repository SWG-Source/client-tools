//======================================================================
//
// CuiStringIdsPlanetMap.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsPlanetMap_H
#define INCLUDED_CuiStringIdsPlanetMap_H

//======================================================================

#include "StringId.h"

//----------------------------------------------------------------------

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace CuiStringIdsPlanetMap
{
	MAKE_STRING_ID (ui_planet_map,  tree_all);
	MAKE_STRING_ID (ui_planet_map,  tooltip_map);
	MAKE_STRING_ID (ui_planet_map,  waiting_locations);
	MAKE_STRING_ID (ui_planet_map,  popup_waypoint_create);
	MAKE_STRING_ID (ui_planet_map,  popup_waypoint_destroy);
	MAKE_STRING_ID (ui_planet_map,  popup_waypoint_activate);
	MAKE_STRING_ID (ui_planet_map,  popup_waypoint_deactivate);
	MAKE_STRING_ID (ui_planet_map,  popup_waypoint_set_name);
	MAKE_STRING_ID (ui_planet_map,  show_zone);
	MAKE_STRING_ID (ui_planet_map,  show_planet);
}

#undef MAKE_STRING_ID

//======================================================================

#endif
