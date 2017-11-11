//======================================================================
//
// CuiStringIdsChat.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsChat_H
#define INCLUDED_CuiStringIdsChat_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace CuiStringIdsChat
{
	MAKE_STRING_ID(ui,      chat_channelid_combat);
	MAKE_STRING_ID(ui,      chat_channelid_gcw);
	MAKE_STRING_ID(ui,      chat_channelid_instantmessage);
	MAKE_STRING_ID(ui,      chat_channelid_none);
	MAKE_STRING_ID(ui,      chat_channelid_planet);
	MAKE_STRING_ID(ui,      chat_channelid_spatial);
	MAKE_STRING_ID(ui,      chat_channelid_systemmessage);
	MAKE_STRING_ID(ui,      chat_channelid_group);
	MAKE_STRING_ID(ui,      chat_channelid_guild);
	MAKE_STRING_ID(ui,      chat_channelid_city);
	MAKE_STRING_ID(ui,      chat_channelid_match_making);
	MAKE_STRING_ID(ui,      chat_channelid_quest);
	MAKE_STRING_ID(ui,      chat_tab_default_recreated);
	MAKE_STRING_ID(ui,      chat_tab_menu_font_size);
	MAKE_STRING_ID(ui,      chat_tab_menu_join_channel);
	MAKE_STRING_ID(ui,      chat_tab_menu_reset_defaults);
	MAKE_STRING_ID(ui,      chat_tab_menu_tab_add);
	MAKE_STRING_ID(ui,      chat_tab_menu_tab_channels);
	MAKE_STRING_ID(ui,      chat_tab_menu_tab_clone);
	MAKE_STRING_ID(ui,      chat_tab_menu_tab_delete);
	MAKE_STRING_ID(ui,      msg_truncated_spatial);

	MAKE_STRING_ID(ui,      chat_tab_confirm_reset_defaults);
}

#undef MAKE_STRING_ID

//======================================================================

#endif
