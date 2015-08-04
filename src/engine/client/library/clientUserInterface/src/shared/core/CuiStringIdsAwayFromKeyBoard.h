//======================================================================
//
// CuiStringIdsAwayFromKeyBoard.h
// Copyright Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsAwayFromKeyBoard_H
#define INCLUDED_CuiStringIdsAwayFromKeyBoard_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------
namespace CuiStringIdsAwayFromKeyBoard
{
	MAKE_STRING_ID(ui_afk, away_from_keyboard_automatic_disabled);
	MAKE_STRING_ID(ui_afk, away_from_keyboard_automatic_off);
	MAKE_STRING_ID(ui_afk, away_from_keyboard_automatic_on);
	MAKE_STRING_ID(ui_afk, away_from_keyboard_automatic_response_prefix);
	MAKE_STRING_ID(ui_afk, away_from_keyboard_automatic_time_many);
	MAKE_STRING_ID(ui_afk, away_from_keyboard_automatic_time_one);
	MAKE_STRING_ID(ui_afk, away_from_keyboard_automatic_time_invalid);
	MAKE_STRING_ID(ui_afk, away_from_keyboard_default_message);
	MAKE_STRING_ID(ui_afk, away_from_keyboard_off);
	MAKE_STRING_ID(ui_afk, away_from_keyboard_on);
};

#undef MAKE_STRING_ID

//======================================================================

#endif // INCLUDED_CuiStringIdsAwayFromKeyBoard_H
