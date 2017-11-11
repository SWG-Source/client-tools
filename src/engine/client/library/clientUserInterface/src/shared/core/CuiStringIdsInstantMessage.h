//======================================================================
//
// CuiStringIdsInstantMessage.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsInstantMessage_H
#define INCLUDED_CuiStringIdsInstantMessage_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------


namespace CuiStringIdsInstantMessage
{
	MAKE_STRING_ID(ui,      im_bad_target_format);
	MAKE_STRING_ID(ui,      im_failed_insufficient_privs_prose);
	MAKE_STRING_ID(ui,      im_failed_unknown_prose);
	MAKE_STRING_ID(ui,      im_message_dupe_discarded_prose);
	MAKE_STRING_ID(ui,      im_messages_sent);
	MAKE_STRING_ID(ui,      im_no_message);
	MAKE_STRING_ID(ui,      im_no_reply_target);
	MAKE_STRING_ID(ui,      im_no_retell_target);
	MAKE_STRING_ID(ui,      im_no_targets);
	MAKE_STRING_ID(ui,      im_recipient_ignored_prose);
	MAKE_STRING_ID(ui,      im_recipient_invalid_prose);
	MAKE_STRING_ID(ui,      im_recipient_offline_prose);
	MAKE_STRING_ID(ui,      im_tell_usage);

	MAKE_STRING_ID(ui,      im_received_brief_prose);
	MAKE_STRING_ID(ui,      im_received_prose);
	MAKE_STRING_ID(ui,      im_success_prose);
	MAKE_STRING_ID(ui,      im_success_reply_prose);

	MAKE_STRING_ID(ui,      im_ttell_no_target);
};

#undef MAKE_STRING_ID

//======================================================================

#endif
