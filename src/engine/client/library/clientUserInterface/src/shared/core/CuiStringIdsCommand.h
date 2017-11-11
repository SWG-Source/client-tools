//======================================================================
//
// CuiStringIdsCommand.h
// Copyright Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsCommand_H
#define INCLUDED_CuiStringIdsCommand_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------
namespace CuiStringIdsCommand
{
	MAKE_STRING_ID(ui_command, command_canceled_prose);

	MAKE_STRING_ID(client, no_valid_target);
	MAKE_STRING_ID(client, echo);
	MAKE_STRING_ID(client, no_paused_commands);
	MAKE_STRING_ID(client, no_ship_targetted);
	MAKE_STRING_ID(client, paused_commands_dumped);

	MAKE_STRING_ID(client, logging_in);

	MAKE_STRING_ID(client, unable_to_execute);
};

#undef MAKE_STRING_ID

//======================================================================

#endif // INCLUDED_CuiStringIdsCommand_H
