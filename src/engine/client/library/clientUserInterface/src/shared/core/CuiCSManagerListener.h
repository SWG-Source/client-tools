//======================================================================
//
// CuiCSManagerListener.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiCSManagerListener_H
#define INCLUDED_CuiCSManagerListener_H

#include "sharedNetworkMessages/CustomerServiceComment.h"
#include "sharedNetworkMessages/CustomerServiceSearchResult.h"
#include "sharedNetworkMessages/GetTicketsResponseMessage.h"

class CustomerServiceCategory;

//======================================================================

class CuiCSManagerListener
{
public:

	struct Messages
	{
		struct ConnectPlayerResponse
		{
			typedef bool Response;
		};

		struct DisconnectPlayerResponse
		{
			typedef bool Response;
		};

		struct CreateTicketResponse
		{
			typedef std::pair<bool, unsigned int> Response;
		};

		struct RequestCategoriesResponse
		{
			typedef stdvector<CustomerServiceCategory>::fwd Response;
		};

		struct SearchKnowledgeBaseResponse
		{
			typedef std::pair<bool, stdvector<CustomerServiceSearchResult>::fwd> Response;
		};

		struct GetArticleResponse
		{
			typedef std::pair<bool, Unicode::String> Response;
		};
	};

	static void install ();
	static void remove  ();
};

//======================================================================

#endif
