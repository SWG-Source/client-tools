//======================================================================
//
// CuiStringIdsTrade.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsTrade_H
#define INCLUDED_CuiStringIdsTrade_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace CuiStringIdsTrade
{
	MAKE_STRING_ID(ui_trade,      aborted);
	MAKE_STRING_ID(ui_trade,      acceptable_other);
	MAKE_STRING_ID(ui_trade,      waiting_complete_prose);
	MAKE_STRING_ID(ui_trade,      requested_prose);
	MAKE_STRING_ID(ui_trade,      request_sent_prose);
	MAKE_STRING_ID(ui_trade,      complete);
	MAKE_STRING_ID(ui_trade,      request_player_busy_prose);
	MAKE_STRING_ID(ui_trade,      request_player_busy_no_obj);
	MAKE_STRING_ID(ui_trade,      request_player_denied_prose);
	MAKE_STRING_ID(ui_trade,      request_player_denied_no_obj);
	MAKE_STRING_ID(ui_trade,      request_player_unreachable_prose);
	MAKE_STRING_ID(ui_trade,      request_player_unreachable_no_obj);
	MAKE_STRING_ID(ui_trade,      add_item_failed_prose);
	MAKE_STRING_ID(ui_trade,      start_fail_target_other_prose);
	MAKE_STRING_ID(ui_trade,      out_of_range_prose);
	MAKE_STRING_ID(ui_trade,      target_lost);
	MAKE_STRING_ID(ui_trade,      err_not_enough_money);
	MAKE_STRING_ID(ui_trade,      err_not_enough_money_prose);
	MAKE_STRING_ID(ui_trade,      start_fail_target_ship);
	MAKE_STRING_ID(ui_trade,      start_fail_target_not_player);
	MAKE_STRING_ID(ui_trade,      target_incapacitated);
	MAKE_STRING_ID(ui_trade,      target_dead);
	MAKE_STRING_ID(ui_trade,      player_incapacitated);
	MAKE_STRING_ID(ui_trade,      player_dead);
};

#undef MAKE_STRING_ID

//======================================================================

#endif
