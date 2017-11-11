//======================================================================
//
// CuiStringIdsWho.h
// Copyright Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsWho_H
#define INCLUDED_CuiStringIdsWho_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------
namespace CuiStringIdsWho
{
	MAKE_STRING_ID(ui_who, anonymous_true);
	MAKE_STRING_ID(ui_who, anonymous_false);
	MAKE_STRING_ID(ui_who, away_from_keyboard);
	MAKE_STRING_ID(ui_who, displaying_faction_rank_true);
	MAKE_STRING_ID(ui_who, displaying_faction_rank_false);
	MAKE_STRING_ID(ui_who, empty);
	MAKE_STRING_ID(ui_who, found_many);
	MAKE_STRING_ID(ui_who, found_one);
	MAKE_STRING_ID(ui_who, found_truncated);
	MAKE_STRING_ID(ui_who, friend_hidden);
	MAKE_STRING_ID(ui_who, friend_list_group_other);
	MAKE_STRING_ID(ui_who, friend_many);
	MAKE_STRING_ID(ui_who, friend_one);
	MAKE_STRING_ID(ui_who, friend_none);
	MAKE_STRING_ID(ui_who, friend_title);
	MAKE_STRING_ID(ui_who, helper);
	MAKE_STRING_ID(ui_who, helper_true);
	MAKE_STRING_ID(ui_who, helper_false);
	MAKE_STRING_ID(ui_who, ignore_many);
	MAKE_STRING_ID(ui_who, ignore_none);
	MAKE_STRING_ID(ui_who, ignore_one);
	MAKE_STRING_ID(ui_who, ignore_title);
	MAKE_STRING_ID(ui_who, link_dead);
	MAKE_STRING_ID(ui_who, looking_for_group);
	MAKE_STRING_ID(ui_who, looking_for_group_true);
	MAKE_STRING_ID(ui_who, looking_for_group_false);
	MAKE_STRING_ID(ui_who, roleplay);
	MAKE_STRING_ID(ui_who, roleplay_true);
	MAKE_STRING_ID(ui_who, roleplay_false);
	MAKE_STRING_ID(ui_who, silenced);
	MAKE_STRING_ID(ui_who, spammered);
	MAKE_STRING_ID(ui_who, spammered_by_me);
	MAKE_STRING_ID(ui_who, squelched);
	MAKE_STRING_ID(ui_who, squelched_by_me);
	MAKE_STRING_ID(ui_who, title);
	MAKE_STRING_ID(ui_who, who_friend);
	MAKE_STRING_ID(ui_who, customer_service_representative);
	MAKE_STRING_ID(ui_who, developer);
	MAKE_STRING_ID(ui_who, warden);
	MAKE_STRING_ID(ui_who, quality_assurance);
	MAKE_STRING_ID(ui_who, outofcharacter);
	MAKE_STRING_ID(ui_who, looking_for_work);
	MAKE_STRING_ID(ui_who, lfw_true);
	MAKE_STRING_ID(ui_who, lfw_false);
};

#undef MAKE_STRING_ID

//======================================================================

#endif // INCLUDED_CuiStringIdsWho_H
