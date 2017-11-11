//======================================================================
//
// CuiStringIdsDroidProgramming.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsDroidProgramming_H
#define INCLUDED_CuiStringIdsDroidProgramming_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace CuiStringIdsDroidProgramming
{
	MAKE_STRING_ID(ui_droid, load_warning);
	MAKE_STRING_ID(ui_droid, unload_warning);
	MAKE_STRING_ID(ui_droid, loaded);
	MAKE_STRING_ID(ui_droid, not_enough_space);
	MAKE_STRING_ID(ui_droid, removed);
	MAKE_STRING_ID(ui_droid, no_changes);
	MAKE_STRING_ID(ui_droid, querying_chip_status);
	MAKE_STRING_ID(ui_droid, changes_message_pre);
	MAKE_STRING_ID(ui_droid, command_added);
	MAKE_STRING_ID(ui_droid, chip_added);
	MAKE_STRING_ID(ui_droid, chip_removed);
	MAKE_STRING_ID(ui_droid, changes_message_post);
};

#undef MAKE_STRING_ID

//======================================================================

#endif
