//======================================================================
//
// CuiSounds.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiSounds_H
#define INCLUDED_CuiSounds_H

//======================================================================

#define MAKE_SOUND(a) const std::string a = #a ;

namespace CuiSounds
{
	MAKE_SOUND (backpack_close);
	MAKE_SOUND (backpack_open);
	MAKE_SOUND (button_arrow_back);
	MAKE_SOUND (button_arrow_forward);
	MAKE_SOUND (button_confirm);
	MAKE_SOUND (button_random);
	MAKE_SOUND (dialog_warning);
	MAKE_SOUND (equip_blaster);
	MAKE_SOUND (hover_move);
	MAKE_SOUND (incoming_im);
	MAKE_SOUND (incoming_mail);
	MAKE_SOUND (increment_big);
	MAKE_SOUND (increment_small);
	MAKE_SOUND (keyboard_clicking);
	MAKE_SOUND (negative);
	MAKE_SOUND (rollover);
	MAKE_SOUND (select);
	MAKE_SOUND (select_info);
	MAKE_SOUND (select_popup);
	MAKE_SOUND (select_rotate);
	MAKE_SOUND (toggle_mouse_mode);
	MAKE_SOUND (radial_start);
	MAKE_SOUND (radial_complete);
	MAKE_SOUND (radial_complete_added);
	MAKE_SOUND (item_zoom_in_st);
	MAKE_SOUND (item_zoom_out_st);
	MAKE_SOUND (combat_action_ok);
	MAKE_SOUND (combat_action_failed);
	MAKE_SOUND (combat_action_warmup);
	MAKE_SOUND (npe2_quest_counter);
	MAKE_SOUND (npe2_quest_step_completed);
};

//======================================================================

#endif
