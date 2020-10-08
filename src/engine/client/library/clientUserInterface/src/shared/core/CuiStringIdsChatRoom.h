//======================================================================
//
// CuiStringIdsChatRoom.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsChatRoom_H
#define INCLUDED_CuiStringIdsChatRoom_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace CuiStringIdsChatRoom
{
	MAKE_STRING_ID(ui_chatroom,      create_create_fail_already_exists_prose);
	MAKE_STRING_ID(ui_chatroom,      create_create_fail_invalid_name_prose);
	MAKE_STRING_ID(ui_chatroom,      create_create_fail_unknown_prose);
	MAKE_STRING_ID(ui_chatroom,      create_err_invalid_name);
	MAKE_STRING_ID(ui_chatroom,      create_err_invalid_root);
	MAKE_STRING_ID(ui_chatroom,      create_err_no_name);
	MAKE_STRING_ID(ui_chatroom,      create_err_no_title);
	MAKE_STRING_ID(ui_chatroom,      create_err_profane_name);
	MAKE_STRING_ID(ui_chatroom,      create_err_profane_title);
	MAKE_STRING_ID(ui_chatroom,      created);
	MAKE_STRING_ID(ui_chatroom,      destroy_fail_prose);
	MAKE_STRING_ID(ui_chatroom,      destroyed_other_prose);
	MAKE_STRING_ID(ui_chatroom,      destroyed_self);
	MAKE_STRING_ID(ui_chatroom,      err_bad_avatar_name);
	MAKE_STRING_ID(ui_chatroom,      err_entered_room_not_found);
	MAKE_STRING_ID(ui_chatroom,      err_no_such_invitee);
	MAKE_STRING_ID(ui_chatroom,      err_no_such_member);
	MAKE_STRING_ID(ui_chatroom,      err_no_such_moderator);
	MAKE_STRING_ID(ui_chatroom,      err_not_found);
	MAKE_STRING_ID(ui_chatroom,      info_creator);
	MAKE_STRING_ID(ui_chatroom,      info_header);
	MAKE_STRING_ID(ui_chatroom,      info_invitees);
	MAKE_STRING_ID(ui_chatroom,      info_members);
	MAKE_STRING_ID(ui_chatroom,      info_moderators);
	MAKE_STRING_ID(ui_chatroom,      info_owner);
	MAKE_STRING_ID(ui_chatroom,      info_private_prefix);
	MAKE_STRING_ID(ui_chatroom,      info_moderated_prefix);
	MAKE_STRING_ID(ui_chatroom,      invitees_header);
	MAKE_STRING_ID(ui_chatroom,      join_fail_invalid_room_id_prose);
	MAKE_STRING_ID(ui_chatroom,      join_fail_invalid_room_name_prose);
	MAKE_STRING_ID(ui_chatroom,      join_fail_not_invited_prose);
	MAKE_STRING_ID(ui_chatroom,      join_fail_unknown_prose);
	MAKE_STRING_ID(ui_chatroom,      join_fail_banned_prose);
	MAKE_STRING_ID(ui_chatroom,      join_fail_swg_chat_server_unavailable_prose);
	MAKE_STRING_ID(ui_chatroom,      join_fail_wrong_faction_prose);
	MAKE_STRING_ID(ui_chatroom,      join_fail_wrong_gcw_region_defender_faction_prose);
	MAKE_STRING_ID(ui_chatroom,      join_fail_invalid_object_prose);
	MAKE_STRING_ID(ui_chatroom,      join_fail_no_game_server_prose);
	MAKE_STRING_ID(ui_chatroom,      join_fail_not_warden_prose);
	MAKE_STRING_ID(ui_chatroom, 	 join_fail_not_guild_leader_prose);
	MAKE_STRING_ID(ui_chatroom,		 join_fail_not_mayor_prose);
	MAKE_STRING_ID(ui_chatroom,      list_received);
	MAKE_STRING_ID(ui_chatroom,      not_a_room);
	MAKE_STRING_ID(ui_chatroom,      not_found);
	MAKE_STRING_ID(ui_chatroom,      ops_header);
	MAKE_STRING_ID(ui_chatroom,      other_entered_prose);
	MAKE_STRING_ID(ui_chatroom,      other_left_prose);
	MAKE_STRING_ID(ui_chatroom,      other_moderator_added_prose);
	MAKE_STRING_ID(ui_chatroom,      other_moderator_removed_prose);
	MAKE_STRING_ID(ui_chatroom,      channel_unavailable);
	MAKE_STRING_ID(ui_chatroom,      query_received);
	MAKE_STRING_ID(ui_chatroom,      self_join);
	MAKE_STRING_ID(ui_chatroom,      self_left);
	MAKE_STRING_ID(ui_chatroom,      self_moderator_added);
	MAKE_STRING_ID(ui_chatroom,      self_moderator_removed);
	MAKE_STRING_ID(ui_chatroom,      send_err_insufficient_privs_prose);
	MAKE_STRING_ID(ui_chatroom,      send_err_no_message);
	MAKE_STRING_ID(ui_chatroom,      send_err_not_a_member);
	MAKE_STRING_ID(ui_chatroom,      send_err_not_a_moderator_prose);
	MAKE_STRING_ID(ui_chatroom,      send_err_unknown_prose);
	MAKE_STRING_ID(ui_chatroom,      who_header);
	MAKE_STRING_ID(ui_chatroom,      err_parser_no_room_specified);
	
	MAKE_STRING_ID(ui_chatroom,      invite_success_prose);
	MAKE_STRING_ID(ui_chatroom,      invite_fail_avatar_not_found_prose);
	MAKE_STRING_ID(ui_chatroom,      invite_fail_room_not_exist_prose);
	MAKE_STRING_ID(ui_chatroom,      invite_fail_room_not_private_prose);
	MAKE_STRING_ID(ui_chatroom,      invite_fail_not_moderator_prose);
	MAKE_STRING_ID(ui_chatroom,      invite_fail_unknown_prose);
	MAKE_STRING_ID(ui_chatroom,      invite_group_success_prose);
	MAKE_STRING_ID(ui_chatroom,      invite_group_fail_avatar_not_found_prose);
	MAKE_STRING_ID(ui_chatroom,      invite_group_fail_unknown_prose);
	MAKE_STRING_ID(ui_chatroom,      uninvite_success_prose);
	MAKE_STRING_ID(ui_chatroom,      uninvite_fail_avatar_not_found_prose);
	MAKE_STRING_ID(ui_chatroom,      uninvite_fail_room_not_exist_prose);
	MAKE_STRING_ID(ui_chatroom,      uninvite_fail_room_not_private_prose);
	MAKE_STRING_ID(ui_chatroom,      uninvite_fail_not_moderator_prose);
	MAKE_STRING_ID(ui_chatroom,      uninvite_fail_avatar_not_invited_prose);
	MAKE_STRING_ID(ui_chatroom,      uninvite_fail_unknown_prose);
		
	MAKE_STRING_ID(ui_chatroom,      mod_add_success_prose);
	MAKE_STRING_ID(ui_chatroom,      mod_add_fail_avatar_not_found_prose);
	MAKE_STRING_ID(ui_chatroom,      mod_add_fail_room_not_exist_prose);
	MAKE_STRING_ID(ui_chatroom,      mod_add_fail_room_not_moderated_prose);
	MAKE_STRING_ID(ui_chatroom,      mod_add_fail_not_moderator_prose);
	MAKE_STRING_ID(ui_chatroom,      mod_add_fail_unknown_prose);
	MAKE_STRING_ID(ui_chatroom,      mod_remove_success_prose);
	MAKE_STRING_ID(ui_chatroom,      mod_remove_fail_avatar_not_found_prose);
	MAKE_STRING_ID(ui_chatroom,      mod_remove_fail_room_not_exist_prose);
	MAKE_STRING_ID(ui_chatroom,      mod_remove_fail_room_not_moderated_prose);
	MAKE_STRING_ID(ui_chatroom,      mod_remove_fail_not_moderator_prose);
	MAKE_STRING_ID(ui_chatroom,      mod_remove_fail_unknown_prose);

	MAKE_STRING_ID(ui_chatroom,      ban_success_prose);
	MAKE_STRING_ID(ui_chatroom,      ban_fail_avatar_not_found_prose);
	MAKE_STRING_ID(ui_chatroom,      ban_fail_room_not_exist_prose);
	MAKE_STRING_ID(ui_chatroom,      ban_fail_unknown_prose);
	MAKE_STRING_ID(ui_chatroom,      ban_fail_no_privs_prose);
	MAKE_STRING_ID(ui_chatroom,      ban_fail_banned_prose);

	MAKE_STRING_ID(ui_chatroom,      unban_success_prose);
	MAKE_STRING_ID(ui_chatroom,      unban_fail_avatar_not_found_prose);
	MAKE_STRING_ID(ui_chatroom,      unban_fail_room_not_exist_prose);
	MAKE_STRING_ID(ui_chatroom,      unban_fail_unknown_prose);
	MAKE_STRING_ID(ui_chatroom,      unban_fail_no_privs_prose);
	MAKE_STRING_ID(ui_chatroom,      unban_fail_banned_prose);

	MAKE_STRING_ID(ui_chatroom,      invited_prose);

	MAKE_STRING_ID(ui_chatroom,      confirm_destroy_room);
	
	MAKE_STRING_ID(ui_chatroom,      kick_success_prose);
	MAKE_STRING_ID(ui_chatroom,      kick_fail_avatar_not_found_prose);
	MAKE_STRING_ID(ui_chatroom,      kick_fail_room_not_exist_prose);
	MAKE_STRING_ID(ui_chatroom,      kick_fail_unknown_prose);
	MAKE_STRING_ID(ui_chatroom,      kick_fail_no_privs_prose);

	MAKE_STRING_ID(ui_chatroom,      chat_spam_limited);
};

#undef MAKE_STRING_ID

//======================================================================

#endif
