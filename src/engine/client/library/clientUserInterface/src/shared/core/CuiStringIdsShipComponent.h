//======================================================================
//
// CuiStringIdsShipComponent.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsShipComponent_H
#define INCLUDED_CuiStringIdsShipComponent_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace CuiStringIdsShipComponent
{
	MAKE_STRING_ID(ui_shipcomponent, engine_speed_max);
	MAKE_STRING_ID(ui_shipcomponent, engine_speed_accel);
	MAKE_STRING_ID(ui_shipcomponent, engine_speed_decel);
	MAKE_STRING_ID(ui_shipcomponent, engine_yaw_accel);
	MAKE_STRING_ID(ui_shipcomponent, engine_pitch_accel);
	MAKE_STRING_ID(ui_shipcomponent, engine_roll_accel);
	MAKE_STRING_ID(ui_shipcomponent, engine_yaw_max);
	MAKE_STRING_ID(ui_shipcomponent, engine_pitch_max);
	MAKE_STRING_ID(ui_shipcomponent, engine_roll_max);

	MAKE_STRING_ID(ui_shipcomponent, chassis_hp);
	MAKE_STRING_ID(ui_shipcomponent, chassis_hp_max);

	MAKE_STRING_ID(ui_shipcomponent, shield_front_hp);
	MAKE_STRING_ID(ui_shipcomponent, shield_front_hp_max);
	MAKE_STRING_ID(ui_shipcomponent, shield_back_hp);
	MAKE_STRING_ID(ui_shipcomponent, shield_back_hp_max);
	MAKE_STRING_ID(ui_shipcomponent, shield_recharge_rate);

	MAKE_STRING_ID(ui_shipcomponent, mass);
	MAKE_STRING_ID(ui_shipcomponent, mass_max);

	MAKE_STRING_ID(ui_shipcomponent, capacitor);
	MAKE_STRING_ID(ui_shipcomponent, capacitor_max);
	MAKE_STRING_ID(ui_shipcomponent, capacitor_recharge_rate);

	MAKE_STRING_ID(ui_shipcomponents, loaded);
	MAKE_STRING_ID(ui_shipcomponents, ammunition);
	MAKE_STRING_ID(ui_shipcomponents, countermeasurepacks);
	MAKE_STRING_ID(ui_shipcomponents, droidSlot);
	MAKE_STRING_ID(ui_shipcomponents, not_certified);
	MAKE_STRING_ID(ui_shipcomponents, err_too_heavy);
	MAKE_STRING_ID(ui_shipcomponents, err_component_not_certified);
	MAKE_STRING_ID(ui_shipcomponents, err_cannot_open_ship_not_certified);
	MAKE_STRING_ID(ui_shipcomponents, err_cannot_open_newbie_ship);
	MAKE_STRING_ID(ui_shipcomponents, err_cannot_open_noncombat_ship);
	MAKE_STRING_ID(ui_shipcomponents, droid);
	MAKE_STRING_ID(ui_shipcomponents, chassis);
	MAKE_STRING_ID(ui_shipcomponents, not_near_terminal);
};

#undef MAKE_STRING_ID

//======================================================================

#endif
