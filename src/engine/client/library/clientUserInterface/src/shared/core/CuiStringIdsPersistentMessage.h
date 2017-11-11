//======================================================================
//
// CuiStringIdsPersistentMessage.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsPersistentMessage_H
#define INCLUDED_CuiStringIdsPersistentMessage_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace CuiStringIdsPersistentMessage
{
	MAKE_STRING_ID(ui_pm,   confirm_empty_mail);
	MAKE_STRING_ID(ui_pm,   delete_all_mail_fail);
	MAKE_STRING_ID(ui_pm,   delete_all_mail_success);
	MAKE_STRING_ID(ui_pm,   err_bad_recipients);
	MAKE_STRING_ID(ui_pm,   err_no_recipients);
	MAKE_STRING_ID(ui_pm,   err_no_subject);
	MAKE_STRING_ID(ui_pm,   err_reply_system);
	MAKE_STRING_ID(ui_pm,   err_waypoint_exists);
	MAKE_STRING_ID(ui_pm,   err_waypoint_matches);
	MAKE_STRING_ID(ui_pm,   failed_unknown_prose);
	MAKE_STRING_ID(ui_pm,   failed_unknown);
	MAKE_STRING_ID(ui_pm,   fwd_prefix);
	MAKE_STRING_ID(ui_pm,   loading_msg);
	MAKE_STRING_ID(ui_pm,   message_bodies_requested);
	MAKE_STRING_ID(ui_pm,   new_message_received);
	MAKE_STRING_ID(ui_pm,   popup_delete);
	MAKE_STRING_ID(ui_pm,   popup_save);
	MAKE_STRING_ID(ui_pm,   popup_save_all);
	MAKE_STRING_ID(ui_pm,   recipient_ignored_prose);
	MAKE_STRING_ID(ui_pm,   recipient_invalid_prose);
	MAKE_STRING_ID(ui_pm,   reply_prefix);
	MAKE_STRING_ID(ui_pm,   returned_mail_prefix);
	MAKE_STRING_ID(ui_pm,   save_succeeded);
};

#undef MAKE_STRING_ID

//======================================================================

#endif
