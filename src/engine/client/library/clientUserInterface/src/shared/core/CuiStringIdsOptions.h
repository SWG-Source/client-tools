//======================================================================
//
// CuiStringIdsOptions.h
// Copyright Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsOptions_H
#define INCLUDED_CuiStringIdsOptions_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace CuiStringIdsOptions
{
	MAKE_STRING_ID(ui_opt, away_from_keyboard_never);
	MAKE_STRING_ID(ui_opt, confirm_modal_chat);
	MAKE_STRING_ID(ui_opt, confirm_modeless_chat);

	MAKE_STRING_ID(ui_opt, confirm_reset_defaults_prose);

	MAKE_STRING_ID(ui_opt_sound, sound_cache_size_16);
	MAKE_STRING_ID(ui_opt_sound, sound_cache_size_32);
	MAKE_STRING_ID(ui_opt_sound, sound_cache_size_64);
	MAKE_STRING_ID(ui_opt_sound, sound_set_provider_error);

	MAKE_STRING_ID(ui_opt, confirm_keymap_reset);
	MAKE_STRING_ID(ui_opt, confirm_keymap_chat_clash);
	MAKE_STRING_ID(ui_opt, invalid_command_type);
};

#undef MAKE_STRING_ID

//======================================================================

#endif
