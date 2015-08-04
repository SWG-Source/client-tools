//======================================================================
//
// CuiStringIdsCustomerService.h
// Copyright Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsCustomerService_H
#define INCLUDED_CuiStringIdsCustomerService_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------
namespace CuiStringIdsCustomerService
{
	MAKE_STRING_ID(ui_serv, cancel_ticket_failure);
	MAKE_STRING_ID(ui_serv, cancel_ticket_success);
	MAKE_STRING_ID(ui_serv, category_community_standard);
	MAKE_STRING_ID(ui_serv, category_harassment);
	MAKE_STRING_ID(ui_serv, category_search_required);
	MAKE_STRING_ID(ui_serv, category_ticket_limit);
	MAKE_STRING_ID(ui_serv, comment_add_fail);
	MAKE_STRING_ID(ui_serv, comment_add_fail_closed);
	MAKE_STRING_ID(ui_serv, comment_add_submitted);
	MAKE_STRING_ID(ui_serv, comment_add_success);
	MAKE_STRING_ID(ui_serv, comment_from);
	MAKE_STRING_ID(ui_serv, comment_id);
	MAKE_STRING_ID(ui_serv, comment_ticket_id);
	MAKE_STRING_ID(ui_serv, create_ticket_failure);
	MAKE_STRING_ID(ui_serv, create_ticket_submitted);
	MAKE_STRING_ID(ui_serv, create_ticket_success);
	MAKE_STRING_ID(ui_serv, create_ticket_waiting_on_response);
	MAKE_STRING_ID(ui_serv, harassment_ticket_limit);
	MAKE_STRING_ID(ui_serv, harassment_verify_failed);
	MAKE_STRING_ID(ui_serv, harassment_verify_needed);
	MAKE_STRING_ID(ui_serv, harassment_verify_success);
	MAKE_STRING_ID(ui_serv, harassment_waiting_for_response);
	MAKE_STRING_ID(ui_serv, know_article_id);
	MAKE_STRING_ID(ui_serv, know_article_no_matches);
	MAKE_STRING_ID(ui_serv, know_article_response_error);
	MAKE_STRING_ID(ui_serv, know_article_retrieve);
	MAKE_STRING_ID(ui_serv, know_article_search);
	MAKE_STRING_ID(ui_serv, know_search_required);
	MAKE_STRING_ID(ui_serv, know_no_article_selected);
	MAKE_STRING_ID(ui_serv, no_categories_found);
	MAKE_STRING_ID(ui_serv, server_no_connection);
	MAKE_STRING_ID(ui_serv, status_closed);
	MAKE_STRING_ID(ui_serv, status_open);
	MAKE_STRING_ID(ui_serv, ticket_click);
	MAKE_STRING_ID(ui_serv, ticket_closed);
	MAKE_STRING_ID(ui_serv, ticket_create_failed_ticket_limit_reached);
	MAKE_STRING_ID(ui_serv, ticket_delete_confirmation);
	MAKE_STRING_ID(ui_serv, ticket_email_message);
	MAKE_STRING_ID(ui_serv, ticket_email_subject);
	MAKE_STRING_ID(ui_serv, ticket_limit_message);
	MAKE_STRING_ID(ui_serv, ticket_modified);
	MAKE_STRING_ID(ui_serv, ticket_none);
	MAKE_STRING_ID(ui_serv, ticket_pending_closed);
	MAKE_STRING_ID(ui_serv, ticket_unread);
	MAKE_STRING_ID(ui_serv, ticket_usage);
	MAKE_STRING_ID(ui_serv, querying_server);
	MAKE_STRING_ID(ui_serv, confirm_cs_browser_spawn)
};

#undef MAKE_STRING_ID

//======================================================================

#endif // INCLUDED_CuiStringIdsCustomerService_H
