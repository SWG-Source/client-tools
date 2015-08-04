//======================================================================
//
// CuiCSManagerListener.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiCSManagerListener.h"

#include "clientGame/CustomerServiceManager.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiStringIdsCustomerService.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/AppendCommentResponseMessage.h"
#include "sharedNetworkMessages/CancelTicketResponseMessage.h"
#include "sharedNetworkMessages/ConnectPlayerResponseMessage.h"
#include "sharedNetworkMessages/CreateTicketResponseMessage.h"
#include "sharedNetworkMessages/GetArticleResponseMessage.h"
#include "sharedNetworkMessages/GetCommentsResponseMessage.h"
#include "sharedNetworkMessages/GetTicketsResponseMessage.h"
#include "sharedNetworkMessages/NewTicketActivityResponseMessage.h"
#include "sharedNetworkMessages/RequestCategoriesResponseMessage.h"
#include "sharedNetworkMessages/SearchKnowledgeBaseResponseMessage.h"

namespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const CuiCSManagerListener::Messages::ConnectPlayerResponse::Response &, CuiCSManagerListener::Messages::ConnectPlayerResponse> 
			connectPlayerResponse;

		MessageDispatch::Transceiver<const CuiCSManagerListener::Messages::DisconnectPlayerResponse::Response &, CuiCSManagerListener::Messages::DisconnectPlayerResponse> 
			disconnectPlayerResponse;

		MessageDispatch::Transceiver<const CuiCSManagerListener::Messages::CreateTicketResponse::Response &, CuiCSManagerListener::Messages::CreateTicketResponse> 
			createTicketResponse;

		MessageDispatch::Transceiver<const CuiCSManagerListener::Messages::RequestCategoriesResponse::Response &, CuiCSManagerListener::Messages::RequestCategoriesResponse> 
			requestCategoriesResponse;

		MessageDispatch::Transceiver<const CuiCSManagerListener::Messages::SearchKnowledgeBaseResponse::Response &, CuiCSManagerListener::Messages::SearchKnowledgeBaseResponse> 
			searchKnowledgeBaseResponse;

		MessageDispatch::Transceiver<const CuiCSManagerListener::Messages::GetArticleResponse::Response &, CuiCSManagerListener::Messages::GetArticleResponse> 
			getArticleResponse;
	}
}

namespace CSAssist
{
	enum CSAssistGameAPIResult                      // Result codes from all API calls
	{
		CSASSIST_RESULT_SUCCESS = 0,                // operation was successful
		CSASSIST_RESULT_TIMEOUT,                    // operation resulted in an API side timeout
		CSASSIST_RESULT_FAIL,                       // general failure
		CSASSIST_RESULT_BADPARAMS,                  // Client passed in bad parameters (strings too long etc)
		CSASSIST_RESULT_OUTOFMEMORY,                // failed to allocate some memory somehow
		CSASSIST_RESULT_NOTICKET,                   // ticket not found, probably illegal ticket ID
		CSASSIST_RESULT_NOCREATE_TICKET,            // failed to create ticket
		CSASSIST_RESULT_UNKNOWN_USER,               // request cannot be made by an anonymous user (user connectCSAssist)
		CSASSIST_RESULT_NOCREATE_COMMENT,           // failed to create ticket comment
		CSASSIST_RESULT_TICKETLOCKED,               // ticked locked by another CSR
		CSASSIST_RESULT_DBERROR,                    // operation failed due to a database problem
		CSASSIST_RESULT_TOOMANYOBJECTS,             // trying to obtain too many tickets/comments!
		CSASSIST_RESULT_INVALID_ACCOUNT,            // unvalidated user, probably wrong login name and/or password
		CSASSIST_RESULT_ACCOUNT_CLOSED,             // validated account but marked closed/banned
		CSASSIST_RESULT_INVALID_DOCUMENTID,         // document ID requested does not exist
		CSASSIST_RESULT_TICKET_XMLFAILED,           // failed to write XML block when creating a ticket.
													// main ticket was still created, so this error *could* be ignored
		CSASSIST_RESULT_ALREADY_REGISTERED,         // player character/UID was already registered for this game/server
													// he will remain registered, but this indicates a potential application bug
		CSASSIST_RESULT_PRIMUS_ERROR                // Primus Knowledge Base returned an error, or malformed XML returned
	};
}

//======================================================================
namespace CSManagerListener
{
	//----------------------------------------------------------------------

	namespace MessageNames
	{
		const char * const AppendCommentResponseMessage         = "AppendCommentResponseMessage";
		const char * const CancelTicketResponseMessage          = "CancelTicketResponseMessage";
		const char * const ConnectPlayerResponseMessage         = "ConnectPlayerResponseMessage";
		const char * const CreateTicketResponseMessage          = "CreateTicketResponseMessage";
		const char * const GetCommentsResponseMessage           = "GetCommentsResponseMessage";
		const char * const GetTicketsResponseMessage            = "GetTicketsResponseMessage";
		const char * const RequestCategoriesResponseMessage     = "RequestCategoriesResponseMessage";
		const char * const SearchKnowledgeBaseResponseMessage   = "SearchKnowledgeBaseResponseMessage";
		const char * const GetArticleResponseMessage            = "GetArticleResponseMessage";
		const char * const NewTicketActivityResponseMessage     = "NewTicketActivityResponseMessage";
	};

	//----------------------------------------------------------------------

	class Listener : 
	public MessageDispatch::Receiver
	{
	public:
		
		//----------------------------------------------------------------------
		
		Listener () :
		MessageDispatch::Receiver ()
		{
			connectToMessage (MessageNames::AppendCommentResponseMessage);
			connectToMessage (MessageNames::CancelTicketResponseMessage);
			connectToMessage (MessageNames::ConnectPlayerResponseMessage);
			connectToMessage (MessageNames::CreateTicketResponseMessage);
			connectToMessage (MessageNames::GetCommentsResponseMessage);
			connectToMessage (MessageNames::GetTicketsResponseMessage);
			connectToMessage (MessageNames::RequestCategoriesResponseMessage);
			connectToMessage (MessageNames::SearchKnowledgeBaseResponseMessage);
			connectToMessage (MessageNames::GetArticleResponseMessage);
			connectToMessage (MessageNames::NewTicketActivityResponseMessage);
		}
		
		//----------------------------------------------------------------------
		
		void receiveMessage(const MessageDispatch::Emitter &, const MessageDispatch::MessageBase & message)
		{
			//----------------------------------------------------------------------
			
			if (message.isType (MessageNames::ConnectPlayerResponseMessage))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				
				const ConnectPlayerResponseMessage msg (ri);

#ifdef _DEBUG
				checkResult(msg.getResult());
#endif // _DEBUG

				bool response = (msg.getResult() == CSAssist::CSASSIST_RESULT_SUCCESS) || (msg.getResult() == CSAssist::CSASSIST_RESULT_ALREADY_REGISTERED);

				Transceivers::connectPlayerResponse.emitMessage(response);
				CustomerServiceManager::requestConnectionResponse(response);
			}
			else if (message.isType (MessageNames::RequestCategoriesResponseMessage))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				
				const RequestCategoriesResponseMessage msg (ri);

#ifdef _DEBUG
				checkResult(msg.getResult());
#endif // _DEBUG

				Transceivers::requestCategoriesResponse.emitMessage(msg.getCategories());
				CustomerServiceManager::requestTicketCategoriesResponse(msg.getCategories());
			}
			else if (message.isType (MessageNames::CreateTicketResponseMessage))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				
				const CreateTicketResponseMessage msg (ri);

#ifdef _DEBUG
				checkResult(msg.getResult());
#endif // _DEBUG

				bool const success = (msg.getResult() == CSAssist::CSASSIST_RESULT_SUCCESS);
				unsigned int ticketId = msg.getTicketId();

				if (   (msg.getResult() == CSAssist::CSASSIST_RESULT_NOCREATE_TICKET)
				    && (ticketId == 0))
				{
					// Ticket creation failed because the ticket limit has been reached

					CuiChatRoomManager::sendPrelocalizedChat(CuiStringIdsCustomerService::ticket_create_failed_ticket_limit_reached.localize());
				}
				else
				{
					Transceivers::createTicketResponse.emitMessage(std::make_pair(success, ticketId));
					CustomerServiceManager::requestTicketCreationResponse(success, ticketId);
				}
			}
			else if (message.isType (MessageNames::AppendCommentResponseMessage))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				
				const AppendCommentResponseMessage msg (ri);

#ifdef _DEBUG
				checkResult(msg.getResult());
#endif // _DEBUG

				bool const success = (msg.getResult() == CSAssist::CSASSIST_RESULT_SUCCESS);
				unsigned int ticketId = msg.getTicketId();

				CustomerServiceManager::requestAppendTicketCommentResponse(success, ticketId);
			}
			else if (message.isType (MessageNames::CancelTicketResponseMessage))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				
				const CancelTicketResponseMessage msg (ri);

#ifdef _DEBUG
				checkResult(msg.getResult());
#endif // _DEBUG

				bool const success = (msg.getResult() == CSAssist::CSASSIST_RESULT_SUCCESS);
				unsigned int ticketId = msg.getTicketId();

				CustomerServiceManager::requestTicketCancellationResponse(success, ticketId);
			}
			else if (message.isType (MessageNames::GetTicketsResponseMessage))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				
				const GetTicketsResponseMessage msg (ri);

#ifdef _DEBUG
				checkResult(msg.getResult());
#endif // _DEBUG

				bool const success = (msg.getResult() == CSAssist::CSASSIST_RESULT_SUCCESS);

				CustomerServiceManager::requestTicketsResponse(success, msg);
			}
			else if (message.isType (MessageNames::GetCommentsResponseMessage))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				
				const GetCommentsResponseMessage msg (ri);

#ifdef _DEBUG
				checkResult(msg.getResult());
#endif // _DEBUG

				bool const success = (msg.getResult() == CSAssist::CSASSIST_RESULT_SUCCESS);

				CustomerServiceManager::requestTicketCommentsResponse(success, msg.getComments());
			}
			else if (message.isType (MessageNames::SearchKnowledgeBaseResponseMessage))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				
				const SearchKnowledgeBaseResponseMessage msg (ri);

#ifdef _DEBUG
				checkResult(msg.getResult());
#endif // _DEBUG

				bool const success = (msg.getResult() == CSAssist::CSASSIST_RESULT_SUCCESS);

				Transceivers::searchKnowledgeBaseResponse.emitMessage(std::make_pair(success, msg.getSearchResults()));
			}
			else if (message.isType (MessageNames::GetArticleResponseMessage))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				
				const GetArticleResponseMessage msg (ri);

#ifdef _DEBUG
				checkResult(msg.getResult());
#endif // _DEBUG

				bool const success = (msg.getResult() == CSAssist::CSASSIST_RESULT_SUCCESS);

				Transceivers::getArticleResponse.emitMessage(std::make_pair(success, msg.getArticle()));
			}
			else if (message.isType (MessageNames::NewTicketActivityResponseMessage))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				
				const NewTicketActivityResponseMessage msg (ri);

				CustomerServiceManager::requestNewTicketActivityResponseMessage(msg.isNewActivity(), msg.getTicketCount());
			}
			else
			{
				DEBUG_REPORT_LOG(true, ("CuiCSManagerListener: Unhandled message\n"));
			}
		}

#ifdef _DEBUG
		void checkResult(int32 const result)
		{
			switch (result)
			{
				case CSAssist::CSASSIST_RESULT_SUCCESS:
					{
						//DEBUG_REPORT_LOG(true, ("CSAssist::CSASSIST_RESULT_SUCCESS\n"));
					}
					break;
				case CSAssist::CSASSIST_RESULT_TIMEOUT:
					{
						DEBUG_REPORT_LOG(true, ("CSAssist::CSASSIST_RESULT_TIMEOUT\n"));
					}
					break;
				case CSAssist::CSASSIST_RESULT_FAIL:
					{
						DEBUG_REPORT_LOG(true, ("CSAssist::CSASSIST_RESULT_FAIL\n"));
					}
					break;
				case CSAssist::CSASSIST_RESULT_BADPARAMS:
					{
						DEBUG_REPORT_LOG(true, ("CSAssist::CSASSIST_RESULT_BADPARAMS\n"));
					}
					break;
				case CSAssist::CSASSIST_RESULT_OUTOFMEMORY:
					{
						DEBUG_REPORT_LOG(true, ("CSAssist::CSASSIST_RESULT_OUTOFMEMORY\n"));
					}
					break;
				case CSAssist::CSASSIST_RESULT_NOTICKET:
					{
						DEBUG_REPORT_LOG(true, ("CSAssist::CSASSIST_RESULT_NOTICKET\n"));
					}
					break;
				case CSAssist::CSASSIST_RESULT_NOCREATE_TICKET:
					{
						DEBUG_REPORT_LOG(true, ("CSAssist::CSASSIST_RESULT_NOCREATE_TICKET\n"));
					}
					break;
				case CSAssist::CSASSIST_RESULT_UNKNOWN_USER:
					{
						DEBUG_REPORT_LOG(true, ("CSAssist::CSASSIST_RESULT_UNKNOWN_USER\n"));
					}
					break;
				case CSAssist::CSASSIST_RESULT_NOCREATE_COMMENT:
					{
						DEBUG_REPORT_LOG(true, ("CSAssist::CSASSIST_RESULT_NOCREATE_COMMENT\n"));
					}
					break;
				case CSAssist::CSASSIST_RESULT_TICKETLOCKED:
					{
						DEBUG_REPORT_LOG(true, ("CSAssist::CSASSIST_RESULT_TICKETLOCKED\n"));
					}
					break;
				case CSAssist::CSASSIST_RESULT_DBERROR:
					{
						DEBUG_REPORT_LOG(true, ("CSAssist::CSASSIST_RESULT_DBERROR\n"));
					}
					break;
				case CSAssist::CSASSIST_RESULT_TOOMANYOBJECTS:
					{
						DEBUG_REPORT_LOG(true, ("CSAssist::CSASSIST_RESULT_TOOMANYOBJECTS\n"));
					}
					break;
				case CSAssist::CSASSIST_RESULT_INVALID_ACCOUNT:
					{
						DEBUG_REPORT_LOG(true, ("CSAssist::CSASSIST_RESULT_INVALID_ACCOUNT\n"));
					}
					break;
				case CSAssist::CSASSIST_RESULT_ACCOUNT_CLOSED:
					{
						DEBUG_REPORT_LOG(true, ("CSAssist::CSASSIST_RESULT_ACCOUNT_CLOSED\n"));
					}
					break;
				case CSAssist::CSASSIST_RESULT_INVALID_DOCUMENTID:
					{
						DEBUG_REPORT_LOG(true, ("CSAssist::CSASSIST_RESULT_INVALID_DOCUMENTID\n"));
					}
					break;
				case CSAssist::CSASSIST_RESULT_TICKET_XMLFAILED:
					{
						DEBUG_REPORT_LOG(true, ("CSAssist::CSASSIST_RESULT_TICKET_XMLFAILED\n"));
					}
					break;
				case CSAssist::CSASSIST_RESULT_ALREADY_REGISTERED:
					{
						DEBUG_REPORT_LOG(true, ("CSAssist::CSASSIST_RESULT_ALREADY_REGISTERED\n"));
					}
					break;
				case CSAssist::CSASSIST_RESULT_PRIMUS_ERROR:
					{
						DEBUG_REPORT_LOG(true, ("CSAssist::CSASSIST_RESULT_PRIMUS_ERROR\n"));
					}
					break;
				default:
					{
						char text[256];
						sprintf(text, "CustomerService: Unhandled ConnectPlayerResponseMessage (%d)", result);
						Game::debugPrintUi(text);
					}
					break;
			}
		}
#else
		void checkResult(int32 const result)
		{
			UNREF(result);
		}
		
#endif // _DEBUG
	};

	Listener * s_listener     = 0;
}

//----------------------------------------------------------------------

void CuiCSManagerListener::install ()
{
	CSManagerListener::s_listener  = new CSManagerListener::Listener;
}

//----------------------------------------------------------------------

void CuiCSManagerListener::remove ()
{
	NOT_NULL (CSManagerListener::s_listener);
	delete CSManagerListener::s_listener;
	CSManagerListener::s_listener = 0;
}

//======================================================================
