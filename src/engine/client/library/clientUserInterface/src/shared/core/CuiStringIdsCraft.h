//======================================================================
//
// CuiStringIdsCraft.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsCraft_H
#define INCLUDED_CuiStringIdsCraft_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------


namespace CuiStringIdsCraft
{
	MAKE_STRING_ID(ui_craft,      confirm_assemble);
	MAKE_STRING_ID(ui_craft,      confirm_complete);
	MAKE_STRING_ID(ui_craft,      confirm_destroy);
	MAKE_STRING_ID(ui_craft,      confirm_exp_done_prose);
	MAKE_STRING_ID(ui_craft,      confirm_cancel);
	MAKE_STRING_ID(ui_craft,      draft_input_item_unusable);
	MAKE_STRING_ID(ui_craft,      draft_input_item_usable);
	MAKE_STRING_ID(ui_craft,      draft_slot_option_loaded_item_prose);
	MAKE_STRING_ID(ui_craft,      draft_slot_option_loaded_resclass_prose);
	MAKE_STRING_ID(ui_craft,      draft_slot_option_req_item_prose);
	MAKE_STRING_ID(ui_craft,      draft_slot_option_req_item_factory_prose);
	MAKE_STRING_ID(ui_craft,      draft_slot_option_req_item_generic_prose);
	MAKE_STRING_ID(ui_craft,      draft_slot_option_req_resclass_prose);
	MAKE_STRING_ID(ui_craft,      draft_slot_options_caption);
	MAKE_STRING_ID(ui_craft,      draft_slot_no_options);
	MAKE_STRING_ID(ui_craft,      draft_slots_waiting_data);
	MAKE_STRING_ID(ui_craft,      err_assembly);
	MAKE_STRING_ID(ui_craft,      err_assembly_slots);
	MAKE_STRING_ID(ui_craft,      err_complete);
	MAKE_STRING_ID(ui_craft,      err_customize);
	MAKE_STRING_ID(ui_craft,      err_experiment);
	MAKE_STRING_ID(ui_craft,      err_finish);
	MAKE_STRING_ID(ui_craft,      err_hopper_output_nonempty);
	MAKE_STRING_ID(ui_craft,      err_no_draft_schematic_category_selected);
	MAKE_STRING_ID(ui_craft,      err_no_draft_schematic_selected);
	MAKE_STRING_ID(ui_craft,      err_no_draft_schematics);
	MAKE_STRING_ID(ui_craft,      err_no_name);
	MAKE_STRING_ID(ui_craft,      err_no_option_selected);
	MAKE_STRING_ID(ui_craft,      err_restart);
	MAKE_STRING_ID(ui_craft,      err_start);
	MAKE_STRING_ID(ui_craft,      err_transfer);
	MAKE_STRING_ID(ui_craft,      err_no_owner);
	MAKE_STRING_ID(ui_craft,      err_not_assembly_stage);
	MAKE_STRING_ID(ui_craft,      err_not_customize_stage);
	MAKE_STRING_ID(ui_craft,      err_no_draft_schematic);
	MAKE_STRING_ID(ui_craft,      err_no_crafting_tool);
	MAKE_STRING_ID(ui_craft,      err_no_manf_schematic);
	MAKE_STRING_ID(ui_craft,      err_invalid_slot);
	MAKE_STRING_ID(ui_craft,      err_invalid_slot_option);
	MAKE_STRING_ID(ui_craft,      err_invalid_ingredient_size);
	MAKE_STRING_ID(ui_craft,      err_slot_full);
	MAKE_STRING_ID(ui_craft,      err_invalid_ingredient);
	MAKE_STRING_ID(ui_craft,      err_ingredient_not_in_inventory);
	MAKE_STRING_ID(ui_craft,      err_cant_remove_resource);
	MAKE_STRING_ID(ui_craft,      err_wrong_resource);
	MAKE_STRING_ID(ui_craft,      err_damaged_component);
	MAKE_STRING_ID(ui_craft,      err_cant_transfer_component);
	MAKE_STRING_ID(ui_craft,      err_wrong_component);
	MAKE_STRING_ID(ui_craft,      err_no_inventory);
	MAKE_STRING_ID(ui_craft,      err_bad_target_hopper);
	MAKE_STRING_ID(ui_craft,      err_bad_target_container);
	MAKE_STRING_ID(ui_craft,      err_empty_slot);
	MAKE_STRING_ID(ui_craft,      err_cant_create_resource_crate);
	MAKE_STRING_ID(ui_craft,      err_empty_slot_assembly);
	MAKE_STRING_ID(ui_craft,      err_partial_slot_assembly);
	MAKE_STRING_ID(ui_craft,      err_no_prototype);
	MAKE_STRING_ID(ui_craft,      err_invalid_crafted_name);
	MAKE_STRING_ID(ui_craft,      err_already_crafting);
	MAKE_STRING_ID(ui_craft,      err_datapad_full_prose);
	MAKE_STRING_ID(ui_craft,      err_ingredient_not_for_slot);
	MAKE_STRING_ID(ui_craft,      err_read_only_draft_schematic);
	MAKE_STRING_ID(ui_craft,      err_invalid_bio_link);
	MAKE_STRING_ID(ui_craft,      err_stacked_loot);
	MAKE_STRING_ID(ui_craft,      wait_assemble);
	MAKE_STRING_ID(ui_craft,      wait_complete_manf_schem);
	MAKE_STRING_ID(ui_craft,      wait_complete_prototype);
	MAKE_STRING_ID(ui_craft,      wait_customize);
	MAKE_STRING_ID(ui_craft,      wait_draft_schematic);
	MAKE_STRING_ID(ui_craft,      wait_draft_slots);
	MAKE_STRING_ID(ui_craft,      wait_experiment);
	MAKE_STRING_ID(ui_craft,      wait_finish);
	MAKE_STRING_ID(ui_craft,      wait_manf_schem);
	MAKE_STRING_ID(ui_craft,      wait_restart_crafting);
	MAKE_STRING_ID(ui_craft,      wait_start_crafting);
	MAKE_STRING_ID(ui_craft,      wait_transfer);
	MAKE_STRING_ID(ui_craft,      wait_send_customization_data);
	MAKE_STRING_ID(ui_craft,      completed_prototype);
	MAKE_STRING_ID(ui_craft,      completed_manf_schem);
	MAKE_STRING_ID(ui_craft,      err_cust_bad_limit);
	MAKE_STRING_ID(ui_craft,      session_ended);
	MAKE_STRING_ID(ui_craft,      tab_inv);
	MAKE_STRING_ID(ui_craft,      tab_hop);

	MAKE_STRING_ID(ui_craft,      complexity_level_0);
	MAKE_STRING_ID(ui_craft,      complexity_level_1);
	MAKE_STRING_ID(ui_craft,      complexity_level_2);

	MAKE_STRING_ID(ui_craft,      draft_attribs_waiting_data);
	MAKE_STRING_ID(ui_craft,      draft_attrib_misc);

	MAKE_STRING_ID(ui_craft,      confirm_exp_done_core_level);
	MAKE_STRING_ID(ui_craft,      confirm_core_level_setting);
};

#undef MAKE_STRING_ID

//======================================================================

#endif
