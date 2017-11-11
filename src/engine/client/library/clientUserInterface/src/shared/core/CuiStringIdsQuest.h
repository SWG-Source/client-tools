//======================================================================
//
// CuiStringIdsQuest.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsQuest_H
#define INCLUDED_CuiStringIdsQuest_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace CuiStringIdsQuest
{
	MAKE_STRING_ID(ui_quest, title);
	MAKE_STRING_ID(ui_quest, description);
	MAKE_STRING_ID(ui_quest, experience);
	MAKE_STRING_ID(ui_quest, faction);
	MAKE_STRING_ID(ui_quest, money);
	MAKE_STRING_ID(ui_quest, credits);
	MAKE_STRING_ID(ui_quest, level);
	MAKE_STRING_ID(ui_quest, acceptance);
	MAKE_STRING_ID(ui_quest, completion);
	MAKE_STRING_ID(ui_quest, pick_a_reward);
	MAKE_STRING_ID(ui_quest, reward);
	MAKE_STRING_ID(ui_quest, target);
	MAKE_STRING_ID(ui_quest, creator);
	MAKE_STRING_ID(ui_quest, startloc);
	MAKE_STRING_ID(ui_quest, destloc);
	MAKE_STRING_ID(ui_quest, efficiency);
	MAKE_STRING_ID(ui_quest, terminal_missions);
	MAKE_STRING_ID(ui_quest, click_to_complete);
	MAKE_STRING_ID(ui_quest, player_quest);
};

#undef MAKE_STRING_ID

//======================================================================

#endif
