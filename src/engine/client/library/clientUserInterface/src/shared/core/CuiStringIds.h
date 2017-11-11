//======================================================================
//
// CuiStringIds.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIds_H
#define INCLUDED_CuiStringIds_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace CuiStringIds
{
	MAKE_STRING_ID(ui,      abort);
	MAKE_STRING_ID(ui,      action_no_target_prose);
	MAKE_STRING_ID(ui,      action_no_target);
	MAKE_STRING_ID(ui,      action_target_not_found_prose);
	MAKE_STRING_ID(ui,      action_target_not_found_id_prose);

	MAKE_STRING_ID(ui,      avatar_create_confirm_cancel);
	MAKE_STRING_ID(ui,      avatar_create);
	MAKE_STRING_ID(ui,      avatar_err_avatar_not_found);
	MAKE_STRING_ID(ui,      avatar_err_no_species_selected);
	MAKE_STRING_ID(ui,      avatar_wait_confirm_create);
	MAKE_STRING_ID(ui,      avatar_wait_random_name);
	MAKE_STRING_ID(ui,      avatar_wait_select_location);
	MAKE_STRING_ID(ui,      avatar_select_location_failed);
	MAKE_STRING_ID(ui,      avatar_wait_transport_location);
	MAKE_STRING_ID(ui,      avatar_err_location_already_selected);
	MAKE_STRING_ID(ui,      avatar_err_name_space);
	MAKE_STRING_ID(ui,      avatar_err_name_none);
	MAKE_STRING_ID(ui,      avatar_err_surname_space);
	MAKE_STRING_ID(ui,      avatar_species_abilities);
	MAKE_STRING_ID(ui,      avatar_location_unavailable);
	MAKE_STRING_ID(ui,      avatar_location_desc_unavailable);
	MAKE_STRING_ID(ui,      avatar_verifying_name);
	

	MAKE_STRING_ID(ui,      cancel);
	MAKE_STRING_ID(ui,      container_none);
	MAKE_STRING_ID(ui,      err_start_location_beta);
	MAKE_STRING_ID(ui,      hairstyle_bald);
	MAKE_STRING_ID(ui,      loading);
	MAKE_STRING_ID(ui,      generatingterrain);
	MAKE_STRING_ID(ui,      loadingobjects);
	MAKE_STRING_ID(ui,      ok);
	MAKE_STRING_ID(ui,      scene_no_scene_file);
	MAKE_STRING_ID(ui,      scene_no_scene_selected);
	MAKE_STRING_ID(ui,      token_and);
	MAKE_STRING_ID(ui,      token_at);
	MAKE_STRING_ID(ui,      token_currency);
	MAKE_STRING_ID(ui,      token_days);
	MAKE_STRING_ID(ui,      token_hours);
	MAKE_STRING_ID(ui,      token_minutes_abbrev);
	MAKE_STRING_ID(ui,      token_seconds_abbrev);
	MAKE_STRING_ID(ui,      token_in_the);
	MAKE_STRING_ID(ui,      token_in);
	MAKE_STRING_ID(ui,      token_on);

	MAKE_STRING_ID(ui,      inv_details_icon);
	MAKE_STRING_ID(ui,      inv_details_name);
	MAKE_STRING_ID(ui,      inv_details_type);
	MAKE_STRING_ID(ui,      inv_details_equipped);
	MAKE_STRING_ID(ui,      inv_details_volume);
	MAKE_STRING_ID(ui,      inv_details_planet);
	MAKE_STRING_ID(ui,      inv_container_suffix_inventory);
	MAKE_STRING_ID(ui,      inv_container_suffix_equipped);
	MAKE_STRING_ID(ui,      inv_object_destroyed_prose);
	MAKE_STRING_ID(ui,      inv_out_of_range_prose);

	MAKE_STRING_ID(ui,      res_hopper_icon);
	MAKE_STRING_ID(ui,      res_hopper_name);
	MAKE_STRING_ID(ui,      res_hopper_amount);
	MAKE_STRING_ID(ui,      res_hopper_efficiency);

	MAKE_STRING_ID(ui,      survey_nothingfound);
	MAKE_STRING_ID(ui,      survey_efficiencytooltipfirst);
	MAKE_STRING_ID(ui,      survey_efficiencytooltipsecond);
	MAKE_STRING_ID(ui,      survey_waypointset);
	MAKE_STRING_ID(ui,      survey_select_resource);
	MAKE_STRING_ID(ui,      lookat_target_lost);
	MAKE_STRING_ID(ui,      tooltip_viewer_3d_controls);

	MAKE_STRING_ID(ui,      confirm_obj_destroy_prose);
	MAKE_STRING_ID(ui,      confirm_exit_game);
	MAKE_STRING_ID(ui,      confirm_train_expertise);
	MAKE_STRING_ID(ui,      confirm_train_expertises);

	MAKE_STRING_ID(ui,      charsheet_homeless);
	MAKE_STRING_ID(ui,      charsheet_married);
	MAKE_STRING_ID(ui,      charsheet_unmarried);
	MAKE_STRING_ID(ui,      charsheet_unknown);
	MAKE_STRING_ID(ui,      charsheet_faction_neutral);
	MAKE_STRING_ID(ui,      charsheet_faction_rebelcovert);
	MAKE_STRING_ID(ui,      charsheet_faction_rebeldeclared);
	MAKE_STRING_ID(ui,      charsheet_faction_imperialcovert);
	MAKE_STRING_ID(ui,      charsheet_faction_imperialdeclared);
	MAKE_STRING_ID(ui,      earth_time);
	MAKE_STRING_ID(ui,      profanity_filter_off);
	MAKE_STRING_ID(ui,      profanity_filter_on);

	MAKE_STRING_ID(ui,      mood_desc_prose);
	MAKE_STRING_ID(ui,      mood_desc_has_animation);

	MAKE_STRING_ID(ui,      chase_camera_on);
	MAKE_STRING_ID(ui,      chase_camera_off);
	MAKE_STRING_ID(ui,      chase_camera_angle_reset);

	MAKE_STRING_ID(ui,      bind_conflict_prose);
	MAKE_STRING_ID(ui,      bind_conflict_cannot_overwrite_prose);
	MAKE_STRING_ID(ui,      bind_delete_cannot_delete_prose);

	MAKE_STRING_ID(ui,      sui_out_of_range_prose);
	MAKE_STRING_ID(ui,      sui_out_of_location_range_prose);
	MAKE_STRING_ID(ui,      sui_obj_lost);

	MAKE_STRING_ID(ui,      permissionlist_title);

	MAKE_STRING_ID(ui,      chair_out_of_range);

	MAKE_STRING_ID(ui,      examine_unguilded);
	MAKE_STRING_ID(ui,      examine_nobadges);
	MAKE_STRING_ID(ui,      examine_fetchingbio);
	MAKE_STRING_ID(ui,      examine_fetchingtitle);
	MAKE_STRING_ID(ui,      examine_fetchingbadges);
	MAKE_STRING_ID(ui,      examine_fetchingguild);
	MAKE_STRING_ID(ui,      examine_title);
	MAKE_STRING_ID(ui,      examine_badges);
	MAKE_STRING_ID(ui,      examine_guild);
	MAKE_STRING_ID(ui,      examine_notitle);

	MAKE_STRING_ID(ui,      macro_nospaces);
	MAKE_STRING_ID(ui,      macro_selecticon);
	MAKE_STRING_ID(ui,      macro_badcommand1);
	MAKE_STRING_ID(ui,      macro_badcommand2);

	MAKE_STRING_ID(ui,      bug_desc);
	MAKE_STRING_ID(ui,      bug_choosetype);
	MAKE_STRING_ID(ui,      bug_chooserepeatable);
	MAKE_STRING_ID(ui,      bug_failed);
	MAKE_STRING_ID(ui,      bug_success);

	MAKE_STRING_ID(ui,      radial_out_of_range_prose);

	MAKE_STRING_ID(ui,      bb_popup_horizontal);
	MAKE_STRING_ID(ui,      bb_popup_vertical);

	MAKE_STRING_ID(ui,      region_entered);
	MAKE_STRING_ID(ui,      region_left);

	MAKE_STRING_ID(ui,      chat_log_enabled);
	MAKE_STRING_ID(ui,      chat_log_disabled);
	MAKE_STRING_ID(ui,      chat_log_file_size_kb);
	MAKE_STRING_ID(ui,      chat_log_file_size_mb);

	MAKE_STRING_ID(ui,      bind_key_prose);

	MAKE_STRING_ID(ui,      active);
	MAKE_STRING_ID(ui,      inactive);

	MAKE_STRING_ID(ui,      waypoint_name);

	MAKE_STRING_ID(ui,      convo_stop_conversing);
	MAKE_STRING_ID(ui,      convo_close_message);

	MAKE_STRING_ID(ui,      spatial_station_pilot);
	MAKE_STRING_ID(ui,      spatial_station_operations);
	MAKE_STRING_ID(ui,      spatial_station_gunner);

	MAKE_STRING_ID(ui,      quest_10_seconds_left);
	MAKE_STRING_ID(ui,      quest_abandon_question);
	MAKE_STRING_ID(ui,      comm_window);

	MAKE_STRING_ID(ui,      confirm_go_home);

	MAKE_STRING_ID(ui,      confirm_give_item_drag_drop);

	MAKE_STRING_ID(ui,      no_trade_tooltip);
	MAKE_STRING_ID(ui,      no_trade_shared_tooltip);
	MAKE_STRING_ID(ui,      unique_tooltip);

	MAKE_STRING_ID(ui,      change_friend_without_saving);
	MAKE_STRING_ID(ui,      close_friend_without_saving);

	MAKE_STRING_ID(ui,      groundplace);
	MAKE_STRING_ID(ui,		groundplace_abort);

	MAKE_STRING_ID(ui,      tcg_exit_confirmation);
};

#undef MAKE_STRING_ID

//======================================================================

#endif
