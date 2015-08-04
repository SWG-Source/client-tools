//======================================================================
//
// CuiStringIdsServer.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsServer_H
#define INCLUDED_CuiStringIdsServer_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace CuiStringIdsServer
{
	MAKE_STRING_ID(ui,      server_avatar_deleted);
	MAKE_STRING_ID(ui,      server_avatar_deleted_failed);
	MAKE_STRING_ID(ui,      server_central_cxn_failed);
	MAKE_STRING_ID(ui,      server_character_creation_max_chars);
	MAKE_STRING_ID(ui,      server_character_creation_refused);
	MAKE_STRING_ID(ui,      server_cluster_address_missing);
	MAKE_STRING_ID(ui,      server_cluster_login_failed);
	MAKE_STRING_ID(ui,      server_confirm_avatar_delete);
	MAKE_STRING_ID(ui,      server_connecting_central);
	MAKE_STRING_ID(ui,      server_connecting_game);
	MAKE_STRING_ID(ui,      server_connecting_login);
	MAKE_STRING_ID(ui,      server_connection_cxn_closed);
	MAKE_STRING_ID(ui,      server_connection_unavailable);
	MAKE_STRING_ID(ui,      server_err_avatar_not_found);
	MAKE_STRING_ID(ui,      server_err_no_character_selected);
	MAKE_STRING_ID(ui,      server_err_no_cluster_selected);
	MAKE_STRING_ID(ui,      server_err_no_username);
	MAKE_STRING_ID(ui,      server_loading_scene);
	MAKE_STRING_ID(ui,      server_login_cxn_failed);
	MAKE_STRING_ID(ui,      server_login_cxn_opened);
	MAKE_STRING_ID(ui,      server_login_cxn_reset);
	MAKE_STRING_ID(ui,      server_wait_avatar_delete);
	MAKE_STRING_ID(ui,      server_online);
	MAKE_STRING_ID(ui,      server_offline);
	MAKE_STRING_ID(ui,      server_timeout_gameserver);
	MAKE_STRING_ID(ui,      server_connection_loading);
	MAKE_STRING_ID(ui,      server_connection_locked);
	MAKE_STRING_ID(ui,      server_connection_restricted);
	MAKE_STRING_ID(ui,      server_loading);
	MAKE_STRING_ID(ui,      server_locked);
	MAKE_STRING_ID(ui,      server_restricted);
	MAKE_STRING_ID(ui,      server_full);
	MAKE_STRING_ID(ui,      server_cluster_full);
	MAKE_STRING_ID(ui,      server_cluster_table_population_very_light);
	MAKE_STRING_ID(ui,      server_cluster_table_population_light);
	MAKE_STRING_ID(ui,      server_cluster_table_population_medium);
	MAKE_STRING_ID(ui,      server_cluster_table_population_heavy);
	MAKE_STRING_ID(ui,      server_cluster_table_population_very_heavy);
	MAKE_STRING_ID(ui,      server_cluster_table_population_extremely_heavy);
	MAKE_STRING_ID(ui,      server_cluster_table_population_full);
	MAKE_STRING_ID(ui,      server_cluster_wait_populate);
	MAKE_STRING_ID(ui,      server_dialog_create_jedi_or_regular);
	MAKE_STRING_ID(ui,      server_dialog_create_jedi);
	MAKE_STRING_ID(ui,      server_please_select_a_profession);
};

#undef MAKE_STRING_ID

//======================================================================

#endif
