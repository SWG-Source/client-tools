//======================================================================
//
// ClientStringIds.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ClientStringIds_H
#define INCLUDED_ClientStringIds_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace ClientStringIds
{
	MAKE_STRING_ID(client,      resource_type_label_prose);
	MAKE_STRING_ID(client,      input_scheme_confirm_reset_prose);
	MAKE_STRING_ID(client,      group_auto_invite_reject_prose);
	MAKE_STRING_ID(client,      group_auto_invite_reject);
	MAKE_STRING_ID(client,      group_invite);
	MAKE_STRING_ID(client,      follow_not_pc_prose);
	MAKE_STRING_ID(client,      follow_start_prose);
	MAKE_STRING_ID(client,      follow_stop_prose);
	MAKE_STRING_ID(client,      enemy_flags);
	MAKE_STRING_ID(client,      no_enemy_flags);
	MAKE_STRING_ID(client,      too_many_commands_queued);
	MAKE_STRING_ID(client,      too_many_commands_queued_generic);
	MAKE_STRING_ID(client,      near_zone_edge);
	MAKE_STRING_ID(client,      autopilot_obstacle);
	MAKE_STRING_ID(client,      group_space_launch_invite);
	MAKE_STRING_ID(client,      group_space_launch_cannot_be_passenger);
	MAKE_STRING_ID(client,      you_cannot_log_out_while_in_combat);
	MAKE_STRING_ID(client,      no_hyperspace_in_tutorial_zone);
};

#undef MAKE_STRING_ID

//======================================================================

#endif
