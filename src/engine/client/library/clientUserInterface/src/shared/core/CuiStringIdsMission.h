//======================================================================
//
// CuiStringIdsMission.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsMission_H
#define INCLUDED_CuiStringIdsMission_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace CuiStringIdsMission
{
	MAKE_STRING_ID(ui_mission,      after_accepting);
	MAKE_STRING_ID(ui_mission,      confirm_accept);
	MAKE_STRING_ID(ui_mission,      confirm_create);
	MAKE_STRING_ID(ui_mission,      confirm_remove);
	MAKE_STRING_ID(ui_mission,      create_anywhere);
	MAKE_STRING_ID(ui_mission,      create_err_no_from);
	MAKE_STRING_ID(ui_mission,      create_err_no_target);
	MAKE_STRING_ID(ui_mission,      create_err_no_to);
	MAKE_STRING_ID(ui_mission,      create_none);
	MAKE_STRING_ID(ui_mission,      faction_declared);
	MAKE_STRING_ID(ui_mission,      faction_symp);
	MAKE_STRING_ID(ui_mission,      friends_list);
	MAKE_STRING_ID(ui_mission,      err_invalid_bond);
	MAKE_STRING_ID(ui_mission,      err_invalid_deadline)
	MAKE_STRING_ID(ui_mission,      err_invalid_limit);
	MAKE_STRING_ID(ui_mission,      err_invalid_payment);
	MAKE_STRING_ID(ui_mission,      err_invalid_target_quantity);
	MAKE_STRING_ID(ui_mission,      err_limit_greater_deadline);
	MAKE_STRING_ID(ui_mission,      err_no_mission_selected);
	MAKE_STRING_ID(ui_mission,      success_rate);
	MAKE_STRING_ID(ui_mission,      table_accepted_by);
	MAKE_STRING_ID(ui_mission,      table_bounty);
	MAKE_STRING_ID(ui_mission,      table_difficulty);
	MAKE_STRING_ID(ui_mission,      table_destination);
	MAKE_STRING_ID(ui_mission,      table_creator);
	MAKE_STRING_ID(ui_mission,      table_payment);
	MAKE_STRING_ID(ui_mission,      table_start);
	MAKE_STRING_ID(ui_mission,      table_target);
	MAKE_STRING_ID(ui_mission,      table_type);
	MAKE_STRING_ID(ui_mission,      table_title);
	MAKE_STRING_ID(ui_mission,      wait_accept);
	MAKE_STRING_ID(ui_mission,      wait_create);
	MAKE_STRING_ID(ui_mission,      wait_details);
	MAKE_STRING_ID(ui_mission,      wait_list);
	MAKE_STRING_ID(ui_mission,      wait_remove);
	MAKE_STRING_ID(ui_mission,      unknown_planet);
	MAKE_STRING_ID(ui_mission,      unknown_target);
	MAKE_STRING_ID(ui_mission,      dynamic_mission);
	MAKE_STRING_ID(ui_mission,      waypoint_activate);
	MAKE_STRING_ID(ui_mission,      waypoint_deactivate);

	MAKE_STRING_ID(ui_mission,      name_destroy);
	MAKE_STRING_ID(ui_mission,      name_deliver);
	MAKE_STRING_ID(ui_mission,      name_bounty);
	MAKE_STRING_ID(ui_mission,      name_entertainer);
	MAKE_STRING_ID(ui_mission,      name_crafting);
	MAKE_STRING_ID(ui_mission,      name_survey);

	MAKE_STRING_ID(ui_mission,      destroy_tab);
	MAKE_STRING_ID(ui_mission,      deliver_tab);
	MAKE_STRING_ID(ui_mission,      bounty_tab);
	MAKE_STRING_ID(ui_mission,      musician_tab);
	MAKE_STRING_ID(ui_mission,      dancer_tab);
	MAKE_STRING_ID(ui_mission,      crafting_tab);
	MAKE_STRING_ID(ui_mission,      survey_tab);
	MAKE_STRING_ID(ui_mission,      hunting_tab);
	MAKE_STRING_ID(ui_mission,      assassin_tab);
	MAKE_STRING_ID(ui_mission,      recon_tab);
	MAKE_STRING_ID(ui_mission,      unknown_reward);
};

#undef MAKE_STRING_ID

//======================================================================

#endif
