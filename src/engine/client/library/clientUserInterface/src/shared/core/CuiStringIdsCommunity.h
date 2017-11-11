//======================================================================
//
// CuiStringIdsCommunity.h
// Copyright Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsCommunity_H
#define INCLUDED_CuiStringIdsCommunity_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------
namespace CuiStringIdsCommunity
{
	MAKE_STRING_ID(ui_cmnty, any);
	MAKE_STRING_ID(ui_cmnty, citizenship_title);
	MAKE_STRING_ID(ui_cmnty, city_gcw_region_defender_title);
	MAKE_STRING_ID(ui_cmnty, guild_gcw_region_defender_title);
	MAKE_STRING_ID(ui_cmnty, clear_preference_confirmation);
	MAKE_STRING_ID(ui_cmnty, clear_profile_confirmation);
	MAKE_STRING_ID(ui_cmnty, friend_comment_success);
	MAKE_STRING_ID(ui_cmnty, friend_comment_usage);
	MAKE_STRING_ID(ui_cmnty, friend_group_success);
	MAKE_STRING_ID(ui_cmnty, friend_group_usage);
	MAKE_STRING_ID(ui_cmnty, friend_invalid);
	MAKE_STRING_ID(ui_cmnty, friend_list_add);
	MAKE_STRING_ID(ui_cmnty, friend_list_hide_offline);
	MAKE_STRING_ID(ui_cmnty, friend_list_modify);
	MAKE_STRING_ID(ui_cmnty, friend_offline);
	MAKE_STRING_ID(ui_cmnty, friend_online);
	MAKE_STRING_ID(ui_cmnty, friend_online_remote);
	MAKE_STRING_ID(ui_cmnty, friend_remove_confirmation);
	MAKE_STRING_ID(ui_cmnty, friend_status_offline);
	MAKE_STRING_ID(ui_cmnty, friend_status_online);
	MAKE_STRING_ID(ui_cmnty, match_found_prose);
	MAKE_STRING_ID(ui_cmnty, no_more_selections);
	MAKE_STRING_ID(ui_cmnty, no_title);
	MAKE_STRING_ID(ui_cmnty, one_selection_only);
	MAKE_STRING_ID(ui_cmnty, quick_match_bad_parameters);
	MAKE_STRING_ID(ui_cmnty, quick_match_many);
	MAKE_STRING_ID(ui_cmnty, quick_match_none);
	MAKE_STRING_ID(ui_cmnty, quick_match_one);
	MAKE_STRING_ID(ui_cmnty, quick_match_title);
	MAKE_STRING_ID(ui_cmnty, search_time_2_minutes);
	MAKE_STRING_ID(ui_cmnty, search_time_5_minutes);
	MAKE_STRING_ID(ui_cmnty, search_time_10_minutes);
	MAKE_STRING_ID(ui_cmnty, search_time_never);
};

#undef MAKE_STRING_ID

//======================================================================

#endif // INCLUDED_CuiStringIdsCommunity_H
