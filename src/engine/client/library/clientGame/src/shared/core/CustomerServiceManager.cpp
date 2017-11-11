// ============================================================================
//
// CustomerServiceManager.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/CustomerServiceManager.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/Species.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiPersistentMessageManager.h"
#include "clientUserInterface/CuiStringIdsCustomerService.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiUtils.h"
#include "clientUserInterface/CuiVoiceChatManager.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ApplicationVersion.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Tag.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/PortalProperty.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/AppendCommentMessage.h"
#include "sharedNetworkMessages/CancelTicketMessage.h"
#include "sharedNetworkMessages/ConnectPlayerMessage.h"
#include "sharedNetworkMessages/CreateTicketMessage.h"
#include "sharedNetworkMessages/CustomerServiceCategory.h"
#include "sharedNetworkMessages/CustomerServiceComment.h"
#include "sharedNetworkMessages/CustomerServiceTicket.h"
#include "sharedNetworkMessages/GetCommentsMessage.h"
#include "sharedNetworkMessages/GetTicketsMessage.h"
#include "sharedNetworkMessages/GetTicketsResponseMessage.h"
#include "sharedNetworkMessages/RequestCategoriesMessage.h"
#include "sharedNetworkMessages/NewTicketActivityMessage.h"
#include "sharedNetworkMessages/VerifyPlayerNameMessage.h"
#include "sharedRandom/Random.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/FileName.h"
#include "unicodeArchive/UnicodeArchive.h"
#include "UIManager.h"

#pragma warning (disable:4503) // decorated name length exceeded, name was truncated

//-----------------------------------------------------------------------------
namespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const CustomerServiceManager::Messages::RequestTicketsResponse::Response &, CustomerServiceManager::Messages::RequestTicketsResponse>
			requestTicketsResponse;

		MessageDispatch::Transceiver<const CustomerServiceManager::Messages::RequestTicketCommentsResponse::Response &, CustomerServiceManager::Messages::RequestTicketCommentsResponse>
			requestTicketCommentsResponse;

		MessageDispatch::Transceiver<const CustomerServiceManager::Messages::RequestTicketCategoriesResponse::Response &, CustomerServiceManager::Messages::RequestTicketCategoriesResponse>
			requestTicketCategoriesResponse;
	}
}

// ============================================================================
//
// CustomerServiceManagerNamespace
//
// ============================================================================

//-----------------------------------------------------------------------------
namespace CustomerServiceManagerNamespace
{
	bool                                       s_installed = false;
	bool                                       s_customerServiceConnected = false;
	float                                      s_connectionRequestTimer = 0.0f;
	float const                                s_connectionRequestTimeoutTime = 10.0f * 60.0f;
	CustomerServiceManager::TicketCategoryList s_ticketCategoryList;
	CustomerServiceManager::TicketCategoryList s_serviceCategoryList;
	CustomerServiceManager::TicketCategoryList s_bugCategoryList;
	CustomerServiceManager::TicketList         s_ticketList;
	CustomerServiceManager::TicketCommentsList s_ticketCommentsList;
	int                                        s_maximumNumberOfCustomerServiceTicketsAllowed = 1;
	Tag const                                  s_tag = TAG(T,I,C,K);

	void remove();
	void sendGetTicketsMessage();
	void sendRequestCategoriesMessage();
	void sendNewTicketActivityMessage();
	CustomerServiceManager::TicketCategoryList buildTicketCategoryList(bool const isBugType, bool const isServiceType);
}

using namespace CustomerServiceManagerNamespace;

//-----------------------------------------------------------------------------
void CustomerServiceManagerNamespace::remove()
{
	DEBUG_FATAL(!s_installed, ("Not installed."));

	s_ticketCategoryList.clear();
	s_serviceCategoryList.clear();
	s_bugCategoryList.clear();

	s_installed = false;
}

//-----------------------------------------------------------------------------
void CustomerServiceManagerNamespace::sendGetTicketsMessage()
{
	DEBUG_REPORT_LOG(true, ("CustomerServiceManagerNamespace::sendGetTicketsMessage() networkId: %s\n", (Game::getPlayer() != NULL) ? Game::getPlayer()->getNetworkId().getValueString().c_str() : "NULL"));

	if (Game::getPlayer() != NULL)
	{
		GameNetwork::send(GetTicketsMessage(0, 1000), true);
	}
}

//-----------------------------------------------------------------------------
void CustomerServiceManagerNamespace::sendRequestCategoriesMessage()
{
	DEBUG_REPORT_LOG(true, ("CustomerServiceManagerNamespace::sendRequestCategoriesMessage() networkId: %s\n", (Game::getPlayer() != NULL) ? Game::getPlayer()->getNetworkId().getValueString().c_str() : "NULL"));

	if (Game::getPlayer() != NULL)
	{
		GameNetwork::send(RequestCategoriesMessage(UIManager::gUIManager().GetLocaleString()), true);
	}
}

//-----------------------------------------------------------------
void CustomerServiceManagerNamespace::sendNewTicketActivityMessage()
{
	DEBUG_REPORT_LOG(true, ("CustomerServiceManagerNamespace::sendNewTicketActivityMessage() networkId: %s\n", (Game::getPlayer() != NULL) ? Game::getPlayer()->getNetworkId().getValueString().c_str() : "NULL"));

	if (Game::getPlayer() != NULL)
	{
		GameNetwork::send(NewTicketActivityMessage(), true);
	}
}

//-----------------------------------------------------------------
CustomerServiceManager::TicketCategoryList CustomerServiceManagerNamespace::buildTicketCategoryList(bool const isBugType, bool const isServiceType)
{
	// Sorted the categories of the requested type

	typedef std::map<Unicode::String, CustomerServiceCategory> SortedSubCategory;
	typedef std::map<Unicode::String, std::pair<CustomerServiceCategory, SortedSubCategory> > SortedCategory;

	SortedCategory sortedCategory;
	SortedSubCategory sortedSubCategory;

	CustomerServiceManager::TicketCategoryList::const_iterator iterTicketCategoryList = s_ticketCategoryList.begin();

	for (; iterTicketCategoryList != s_ticketCategoryList.end(); ++iterTicketCategoryList)
	{
		CustomerServiceCategory const &category = (*iterTicketCategoryList);

		//DEBUG_REPORT_LOG(true, ("Category - %s %d isBugType: %s isServiceType: %s\n", category.m_categoryName.c_str(), category.m_categoryId, category.m_isBugType ? "yes" : "no", category.m_isServiceType ? "yes" : "no"));

		if (!category.m_subCategories.empty())
		{
			CustomerServiceCategory newCategory(category.m_categoryName, category.m_categoryId, isBugType, isServiceType);

			// See if any of the sub categories are for type service

			CustomerServiceManager::TicketCategoryList::const_iterator iterSubCategoryList = category.m_subCategories.begin();
			sortedSubCategory.clear();

			for (; iterSubCategoryList != category.m_subCategories.end(); ++iterSubCategoryList)
			{
				CustomerServiceCategory const &subCategory = (*iterSubCategoryList);

				//DEBUG_REPORT_LOG(true, ("   SubCategory - %s %d isBugType: %s isServiceType: %s\n", subCategory.m_categoryName.c_str(), subCategory.m_categoryId, subCategory.m_isBugType ? "yes" : "no", subCategory.m_isServiceType ? "yes" : "no"));

				CustomerServiceCategory newSubCategory(subCategory.m_categoryName, subCategory.m_categoryId, false, false);

				if (   isServiceType
				    && subCategory.m_isServiceType)
				{
					newSubCategory.m_isServiceType = true;
					Unicode::String const & subCategoryName = newSubCategory.getCategoryName();
					sortedSubCategory.insert(std::make_pair(subCategoryName, newSubCategory));
				}

				if (   isBugType
				    && subCategory.m_isBugType)
				{
					newSubCategory.m_isBugType = true;
					Unicode::String const & subCategoryName = newSubCategory.getCategoryName();
					sortedSubCategory.insert(std::make_pair(subCategoryName, newSubCategory));
				}
			}

			if (!sortedSubCategory.empty())
			{
				Unicode::String const & categoryName = newCategory.getCategoryName();
				sortedCategory.insert(std::make_pair(categoryName, std::make_pair(newCategory, sortedSubCategory)));
			}
		}
	}

	// Put the sorted categories back into a vector

	CustomerServiceManager::TicketCategoryList result;

	SortedCategory::const_iterator iterSortedCategory = sortedCategory.begin();

	for (; iterSortedCategory != sortedCategory.end(); ++iterSortedCategory)
	{
		CustomerServiceCategory customerServiceCategory;

		customerServiceCategory = iterSortedCategory->second.first;
		customerServiceCategory.m_subCategories.clear();

		// Get the sub-categories

		SortedSubCategory::const_iterator iterSortedSubCategory = iterSortedCategory->second.second.begin();

		for (; iterSortedSubCategory != iterSortedCategory->second.second.end(); ++iterSortedSubCategory)
		{
			customerServiceCategory.m_subCategories.push_back(iterSortedSubCategory->second);
		}

		result.push_back(customerServiceCategory);
	}

	return result;
}

// ============================================================================
//
// CustomerServiceManager
//
// ============================================================================

//-----------------------------------------------------------------------------
void CustomerServiceManager::install()
{
	InstallTimer const installTimer("CustomerServiceManager::install");

	DEBUG_FATAL(s_installed, ("Already installed."));

	s_installed = true;
	s_maximumNumberOfCustomerServiceTicketsAllowed = ConfigFile::getKeyInt ("ClientGame", "maximumNumberOfCustomerServiceTicketsAllowed", 1);

	ExitChain::add(CustomerServiceManagerNamespace::remove, "CustomerServiceManager::remove", 0, false);
}

//-----------------------------------------------------------------------------
void CustomerServiceManager::alter(float const deltaTime)
{
	// We must connect each player to the CS server when they login to make
	// sure they do not have any tickets that have been responded to by a CSR.

	if (Game::getPlayer() != NULL)
	{
		s_connectionRequestTimer -= deltaTime;

		if (   !Game::getSinglePlayer()
		    && !s_customerServiceConnected
		    && (s_connectionRequestTimer < 0.0f))
		{
			requestConnection();
		}
	}
}

//-----------------------------------------------------------------------------
int CustomerServiceManager::getMaximumNumberOfCustomerServiceTicketsAllowed()
{
	return s_maximumNumberOfCustomerServiceTicketsAllowed;
}

//-----------------------------------------------------------------------------
void CustomerServiceManager::requestConnection()
{
	DEBUG_REPORT_LOG(true, ("CustomerServiceManager::requestConnection() networkId: %s\n", (Game::getPlayer() != NULL) ? Game::getPlayer()->getNetworkId().getValueString().c_str() : "NULL"));

	if (   (Game::getPlayer() != NULL)
	    && !s_customerServiceConnected
	    && (s_connectionRequestTimer < 0.0f))
	{
		GameNetwork::send(ConnectPlayerMessage(), true);

		s_connectionRequestTimer = s_connectionRequestTimeoutTime;
	}
}

//-----------------------------------------------------------------------------
void CustomerServiceManager::requestConnectionResponse(bool const connected)
{
	DEBUG_REPORT_LOG(true, ("CustomerServiceManager::requestConnectionResponse() connected: %s\n", connected ? "yes" : "no"));

	if (connected)
	{
		// Only request categories if there exists no categories

		if (s_ticketCategoryList.empty())
		{
			sendRequestCategoriesMessage();
		}

		if (!s_customerServiceConnected)
		{
			sendNewTicketActivityMessage();
		}

		s_customerServiceConnected = true;
	}
}

//-----------------------------------------------------------------
void CustomerServiceManager::requestNewTicketActivityResponseMessage(bool const newActivity, unsigned int const ticketCount)
{
	UNREF(newActivity);
	DEBUG_REPORT_LOG(true, ("CustomerServiceManager::requestNewTicketActivityResponseMessage() activity: %s ticketCount: %i\n", newActivity ? "yes" : "no", ticketCount));

	if (ticketCount > 0)
	{
		// If we have some tickets, lets make sure and get them

		sendGetTicketsMessage();
	}
}

//-----------------------------------------------------------------
void CustomerServiceManager::requestTicketsResponse(bool const success, GetTicketsResponseMessage const &response)
{
	DEBUG_REPORT_LOG(true, ("CustomerServiceManager::requestTicketsResponse() count: %i success: %s\n", response.getTotalNumTickets(), success ? "yes" : "no"));

	if (success)
	{
		s_ticketList = response.getTickets();

		Transceivers::requestTicketsResponse.emitMessage(true);

		ClientObject const * const clientObject = Game::getClientPlayer();

		if (clientObject != NULL)
		{
			Unicode::String body;
			int unreadTicketCount = 0;

			// See if any tickets are unread

			TicketList::const_iterator iterTicketList = s_ticketList.begin();

			for (; iterTicketList != s_ticketList.end(); ++iterTicketList)
			{
				if (!iterTicketList->isRead())
				{
					++unreadTicketCount;
					CuiStringVariablesData data;
					data.digit_i = static_cast<int>(iterTicketList->getTicketId());
					data.targetName = Unicode::narrowToWide(iterTicketList->getCharacterName());
					Unicode::String unreadTicketString;
					CuiStringVariablesManager::process(CuiStringIdsCustomerService::ticket_email_message, data, unreadTicketString);

					body += unreadTicketString;
					body += Unicode::narrowToWide("\n");
				}
			}

			if (unreadTicketCount > 0)
			{
				// Just get the first name

				std::string recipient;
				Unicode::String const &playerName = clientObject->getLocalizedName();
				std::string::size_type spacePos = playerName.find(' ');

				if (spacePos != std::string::npos)
				{
					recipient = Unicode::wideToNarrow(playerName.substr(0, spacePos));
				}
				else
				{
					recipient = Unicode::wideToNarrow(playerName);
				}

				Unicode::String subject(CuiStringIdsCustomerService::ticket_email_subject.localize());
				CuiPersistentMessageManager::sendMessage(recipient, subject, body, Unicode::emptyString);
			}
		}
	}
}

//-----------------------------------------------------------------
void CustomerServiceManager::requestTicketCategoriesResponse(TicketCategoryList const &ticketCategoryList)
{
	DEBUG_REPORT_LOG(true, ("CustomerServiceManager::requestTicketCategoriesResponse() all category count: %i\n", ticketCategoryList.size()));
	s_ticketCategoryList = ticketCategoryList;

	// Build the service category list

	s_serviceCategoryList = buildTicketCategoryList(false, true);
	DEBUG_REPORT_LOG(true, ("CustomerServiceManager::requestTicketCategoriesResponse() service category count: %i\n", s_serviceCategoryList.size()));

	// Build the bug category list

	s_bugCategoryList = buildTicketCategoryList(true, false);
	DEBUG_REPORT_LOG(true, ("CustomerServiceManager::requestTicketCategoriesResponse() bug category count: %i\n", s_bugCategoryList.size()));

	Transceivers::requestTicketCategoriesResponse.emitMessage(true);
}

//-----------------------------------------------------------------
void CustomerServiceManager::ticketChangeResponseMessage(unsigned int const ticketId)
{
	DEBUG_REPORT_LOG(true, ("CustomerServiceManager::ticketChangeResponseMessage\n"));

	CuiStringVariablesData data;
	data.digit_i = ticketId;
	Unicode::String modifiedTicketString;
	CuiStringVariablesManager::process(CuiStringIdsCustomerService::ticket_modified, data, modifiedTicketString);
	CuiChatRoomManager::sendPrelocalizedChat(modifiedTicketString);
}

//-----------------------------------------------------------------
void CustomerServiceManager::requestTicketCreation(std::string const &playerName, unsigned int const category, unsigned int const subCategory, Unicode::String const &details, Unicode::String const &harassingPlayerName)
{
	DEBUG_REPORT_LOG(true, ("CustomerServiceManager::requestTicketCreation() playerName(%s) category(%i) sub-category(%i) details(%s) harassing player(%s)\n", playerName.c_str(), category, subCategory, Unicode::wideToNarrow(details).c_str(), Unicode::wideToNarrow(harassingPlayerName).c_str()));

	if (Game::getPlayer() != NULL)
	{
		GameNetwork::send(CreateTicketMessage(playerName, category, subCategory, details, Unicode::narrowToWide(CustomerServiceManager::getPlayerInformation()), harassingPlayerName, UIManager::gUIManager().GetLocaleString()), true);

		// Create a normal report (sent to the LOG server) if this is a harassment ticket

		if (!harassingPlayerName.empty())
		{
			uint32 const hash = Crc::normalizeAndCalculate("report");

			IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash, NetworkId::cms_invalid, harassingPlayerName));
		}
	}
}

//-----------------------------------------------------------------
void CustomerServiceManager::requestTicketCreationResponse(bool const success, unsigned int const ticketId)
{
	DEBUG_REPORT_LOG(true, ("CustomerServiceManager::requestTicketCreationResponse() ticket(%i) success(%s)\n", ticketId, success ? "yes" : "no"));

	if (success)
	{
		sendGetTicketsMessage();

		CuiStringVariablesData data;
		data.digit_i = static_cast<int>(ticketId);

		Unicode::String createTicketSuccessResponse;
		CuiStringVariablesManager::process(CuiStringIdsCustomerService::create_ticket_success, data, createTicketSuccessResponse);

		CuiChatRoomManager::sendPrelocalizedChat(createTicketSuccessResponse);
	}
	else
	{
		CuiChatRoomManager::sendPrelocalizedChat(CuiStringIdsCustomerService::create_ticket_failure.localize());
	}
}

//-----------------------------------------------------------------
void CustomerServiceManager::requestTicketComments(unsigned int ticketId)
{
	DEBUG_REPORT_LOG(true, ("CustomerServiceManager::requestTicketComments() ticket(%i)\n", ticketId));

	if (Game::getPlayer() != NULL)
	{
		GameNetwork::send(GetCommentsMessage(ticketId), true);

		// Remove the unread status of the ticket

		bool ticketFound = false;
		TicketList::iterator iterTicketList = s_ticketList.begin();

		for (; iterTicketList != s_ticketList.end(); ++iterTicketList)
		{
			CustomerServiceTicket &ticket = (*iterTicketList);

			if (ticket.m_ticketId == ticketId)
			{
				ticketFound = true;
				ticket.m_read = true;
				break;
			}
		}

		// Send a new list of tickets with the new unread status

		if (ticketFound)
		{
			Transceivers::requestTicketsResponse.emitMessage(true);
		}
	}
}

//-----------------------------------------------------------------
void CustomerServiceManager::requestTicketCommentsResponse(bool const success, TicketCommentsList const &comments)
{
	DEBUG_REPORT_LOG(true, ("CustomerServiceManager::requestTicketCommentsResponse() success: %s\n", success ? "yes" : "no"));

	if (success)
	{
		s_ticketCommentsList = comments;

		Transceivers::requestTicketCommentsResponse.emitMessage(true);
	}
}

//-----------------------------------------------------------------
void CustomerServiceManager::requestTicketCancellation(unsigned int const ticketId, bool const emitTransceiver /* = true */)
{
	DEBUG_REPORT_LOG(true, ("CustomerServiceManager::requestTicketCancellation() ticket: %i\n", ticketId));

	if (Game::getPlayer() != NULL)
	{
		GameNetwork::send(CancelTicketMessage(ticketId, Unicode::emptyString), true);

		bool found = false;
		TicketList::iterator iterTicketList = s_ticketList.begin();

		for (; iterTicketList != s_ticketList.end(); ++iterTicketList)
		{
			if (iterTicketList->getTicketId() == ticketId)
			{
				found = true;

				// Delete the ticket from the list

				s_ticketList.erase(iterTicketList);

				break;
			}
		}

		// If the ticket was found, then update any UIs that the ticket was
		// removed

		if (   emitTransceiver
		    && found)
		{
			Transceivers::requestTicketsResponse.emitMessage(true);
			DEBUG_REPORT_LOG(true, ("CustServ: Cancelling ticket(%d) tickets left(%d)\n", ticketId, static_cast<int>(s_ticketList.size())));
		}
	}
}

//-----------------------------------------------------------------
void CustomerServiceManager::requestTicketCancellationResponse(bool const success, unsigned int const ticketId)
{
	DEBUG_REPORT_LOG(true, ("CustomerServiceManager::requestCancelTicketResponse() ticket: %i success: %s\n", ticketId, success ? "yes" : "no"));

	if (success)
	{
		CuiStringVariablesData data;
		data.digit_i = static_cast<int>(ticketId);

		Unicode::String cancelTicketSuccessResponse;
		CuiStringVariablesManager::process(CuiStringIdsCustomerService::cancel_ticket_success, data, cancelTicketSuccessResponse);

		CuiChatRoomManager::sendPrelocalizedChat(cancelTicketSuccessResponse);
	}
	else
	{
		CuiStringVariablesData data;
		data.digit_i = static_cast<int>(ticketId);

		Unicode::String cancelTicketFailureResponse;
		CuiStringVariablesManager::process(CuiStringIdsCustomerService::cancel_ticket_failure, data, cancelTicketFailureResponse);

		CuiChatRoomManager::sendPrelocalizedChat(cancelTicketFailureResponse);

		// Get a fresh list of tickets, since we previously removed
		// the ticket from the list

		sendGetTicketsMessage();
	}
}

//-----------------------------------------------------------------
void CustomerServiceManager::requestAppendTicketComment(unsigned int ticketId, std::string const &playerName, Unicode::String const &comment)
{
	DEBUG_REPORT_LOG(true, ("CustomerServiceManager::requestAppendTicketComment() ticket: %i playerName: %s comment: %s\n", ticketId, playerName.c_str(), Unicode::wideToNarrow(comment).c_str()));

	// Ensure the ticket id is still valid in the player's ticket list

	bool found = false;
	TicketList::const_iterator iterTicketList = s_ticketList.begin();

	for (; iterTicketList != s_ticketList.end(); ++iterTicketList)
	{
		unsigned int const ticketId = iterTicketList->getTicketId();

		if (iterTicketList->getTicketId() == ticketId)
		{
			found = true;
			break;
		}
	}

	if (found)
	{
		if (Game::getPlayer() != NULL)
		{
			GameNetwork::send(AppendCommentMessage(ticketId, playerName, comment), true);
		}
	}
	else
	{
		DEBUG_WARNING(true, ("A ticket append comment was requested for a ticket that no longer exists."));
	}
}

//-----------------------------------------------------------------
void CustomerServiceManager::requestAppendTicketCommentResponse(bool const success, unsigned int const ticketId)
{
	DEBUG_REPORT_LOG(true, ("CustomerServiceManager::requestAppendTicketCommentResponse() ticket: %i success: %s\n", ticketId, success ? "yes" : "no"));

	Unicode::String addCommentResponse;
	CuiStringVariablesData data;
	data.digit_i = static_cast<int>(ticketId);

	if (success)
	{
		CuiStringVariablesManager::process(CuiStringIdsCustomerService::comment_add_success, data, addCommentResponse);

		// Request the new comment for this ticket

		CustomerServiceManager::requestTicketComments(ticketId);
	}
	else
	{
		CuiStringVariablesManager::process(CuiStringIdsCustomerService::comment_add_fail, data, addCommentResponse);
	}

	CuiChatRoomManager::sendPrelocalizedChat(addCommentResponse);
}

//-----------------------------------------------------------------
CustomerServiceManager::TicketCategoryList const &CustomerServiceManager::getServiceCategoryList()
{
	return s_serviceCategoryList;
}

//-----------------------------------------------------------------
CustomerServiceManager::TicketCategoryList const &CustomerServiceManager::getBugCategoryList()
{
	return s_bugCategoryList;
}

//-----------------------------------------------------------------
CustomerServiceManager::TicketList const &CustomerServiceManager::getTicketList()
{
	return s_ticketList;
}

//-----------------------------------------------------------------
CustomerServiceManager::TicketCommentsList const &CustomerServiceManager::getTicketCommentsList()
{
	return s_ticketCommentsList;
}

//-----------------------------------------------------------------
bool CustomerServiceManager::getCategoryText(int const categoryId, Unicode::String &categoryString)
{
	bool result = false;

	// Find the category

	TicketCategoryList::const_iterator iterTicketCategoryList = s_ticketCategoryList.begin();

	for (; iterTicketCategoryList != s_ticketCategoryList.end(); ++iterTicketCategoryList)
	{
		int const currentCategoryId = iterTicketCategoryList->getCategoryId();

		if (currentCategoryId == categoryId)
		{
			result = true;
			categoryString = iterTicketCategoryList->getCategoryName();
			break;
		}
	}

	return result;
}

//-----------------------------------------------------------------
bool CustomerServiceManager::getSubCategoryText(int const categoryId, int const subCategoryId, Unicode::String &subCategoryString)
{
	bool result = false;

	// Find the category

	TicketCategoryList::const_iterator iterTicketCategoryList = s_ticketCategoryList.begin();

	for (; iterTicketCategoryList != s_ticketCategoryList.end(); ++iterTicketCategoryList)
	{
		int const currentCategoryId = iterTicketCategoryList->getCategoryId();

		if (currentCategoryId == categoryId)
		{
			// Find the sub-category

			std::vector<CustomerServiceCategory>::const_iterator iterTicketSubCategoryList = iterTicketCategoryList->getSubCategories().begin();

			for (; iterTicketSubCategoryList != iterTicketCategoryList->getSubCategories().end(); ++iterTicketSubCategoryList)
			{
				int const currentSubCategoryId = iterTicketSubCategoryList->getCategoryId();

				if (currentSubCategoryId == subCategoryId)
				{
					result = true;
					subCategoryString = iterTicketSubCategoryList->getCategoryName();
					break;
				}
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------
bool CustomerServiceManager::isConnected()
{
	return s_customerServiceConnected;
}

//-----------------------------------------------------------------
void CustomerServiceManager::gameStart()
{
	DEBUG_REPORT_LOG(true, ("CustomerServiceManager::gameStart()\n"));

	s_customerServiceConnected = false;
	s_connectionRequestTimer = 0.0f;
}

//-----------------------------------------------------------------
std::string CustomerServiceManager::getPlayerInformation()
{
	std::string result;
	const ClientObject* const player = Game::getClientPlayer();

	if (player != NULL)
	{
		char buf[512];

		//get the world position of the player
		const Vector & world_pos = player->getPosition_w ();
		const int world_heading = static_cast<int> (convertRadiansToDegrees (player->getObjectFrameK_w ().theta ()));
		snprintf (buf, sizeof(buf), "World Location: (%6.2f, %6.2f, %6.2f) Heading: %3i\n", world_pos.x, world_pos.y, world_pos.z, world_heading);
		buf[sizeof(buf) - 1] = '\0';
		result += buf;

		//get the cell/building position and information, if any
		const CellProperty* const cp = player->getParentCell();

		if (cp && cp != CellProperty::getWorldCellProperty())
		{
			const Vector & cell_pos = player->getPosition_p ();
			const int cell_heading = static_cast<int> (convertRadiansToDegrees (player->getObjectFrameK_p ().theta ()));
			const char *cellName = cp->getCellName();

			if(!cellName)
			{
				cellName = "Unknown";
			}

			snprintf(buf, sizeof(buf), "Cell Name: %s\n", cellName);
			buf[sizeof(buf) - 1] = '\0';
			result += buf;

			const Object *cellApp = cp->getAppearanceObject();
			if(cellApp)
			{
				const NetworkId& nid = cellApp ->getNetworkId();
				snprintf(buf, sizeof(buf), "Cell NetworkId: %s\n", nid.getValueString().c_str());
				buf[sizeof(buf) - 1] = '\0';
				result += buf;

				const PortalProperty* portalProp = cp->getPortalProperty();
				if(portalProp)
				{
					const Object& building = portalProp->getOwner();
					const char* name = building.getObjectTemplateName();
					if(name)
					{
						snprintf(buf, sizeof(buf), "Cell's Owner Template Name: %s\n", name);
						buf[sizeof(buf) - 1] = '\0';
						result += buf;
					}

					const NetworkId& buildingNid = building.getNetworkId();
					snprintf(buf, sizeof(buf), "Cell's Owner NetworkId: %s\n", buildingNid.getValueString().c_str());
					buf[sizeof(buf) - 1] = '\0';
					result += buf;
				}
			}

			snprintf(buf, sizeof(buf), "Position in cell: %6.2f %6.2f %6.2f %3i\n", cell_pos.x, cell_pos.y, cell_pos.z, cell_heading);
			buf[sizeof(buf) - 1] = '\0';
			result += buf;
		}

		//get planet
		const TerrainObject* const terrain = TerrainObject::getConstInstance();

		if (terrain)
		{
			const std::string terrainFileName = terrain->getAppearance()->getAppearanceTemplate()->getName();
			FileName f (FileName::P_terrain, terrainFileName.c_str());
			f.stripPathAndExt();
			snprintf(buf, sizeof(buf), "Planet: %s\n", static_cast<const char*>(f));
			buf[sizeof(buf) - 1] = '\0';
			result += buf;
		}

		const std::string clusterName = GameNetwork::getCentralServerName();
		snprintf(buf, sizeof(buf), "Cluster: %s\n", clusterName.c_str());
		buf[sizeof(buf) - 1] = '\0';
		result += buf;

		//get character name/species/gender
		const std::string playerName = Unicode::wideToNarrow(player->getLocalizedName());
		const ObjectTemplate* const t = player->getObjectTemplate();
		const SharedCreatureObjectTemplate* const c_t = NON_NULL(dynamic_cast<const SharedCreatureObjectTemplate*>(t));
		const SharedCreatureObjectTemplate::Species species = c_t->getSpecies();
		const SharedCreatureObjectTemplate::Gender gender = c_t->getGender();
		const std::string speciesString = Unicode::wideToNarrow(Species::getLocalizedName(species));

		std::string genderString;

		switch (gender)
		{
			case SharedCreatureObjectTemplate::GE_male:   { genderString = "Male"; } break;
			case SharedCreatureObjectTemplate::GE_female: { genderString = "Female"; } break;
			case SharedCreatureObjectTemplate::GE_other:  { genderString = "Other"; } break;
			default:                                      { genderString = "Unknown gender"; } break;
		} //lint !e788 Gender_Last not used in switch statement, can't use it since it == GE_Other and you can't reuse case statements

		snprintf(buf, sizeof(buf), "Character: %s (%s %s)\n", playerName.c_str(), speciesString.c_str(), genderString.c_str());
		buf[sizeof(buf) - 1] = '\0';
		result += buf;

		//get game version
		std::string gameVersion = ApplicationVersion::getInternalVersion();
		snprintf(buf, sizeof(buf), "Game Version: %s\n", gameVersion.c_str());
		buf[sizeof(buf) - 1] = '\0';
		result += buf;

		//get date/time
		Unicode::String timeStamp;
		CuiUtils::FormatDate(timeStamp, CuiUtils::GetSystemSeconds());
		snprintf(buf, sizeof(buf), "Date/Time: %s\n", Unicode::wideToNarrow(timeStamp).c_str());
		buf[sizeof(buf) - 1] = '\0';
		result += buf;

		//get voice chat information
		result += CuiVoiceChatManager::getCsReportString();
	}

	//DEBUG_REPORT_LOG(true, ("CustomerServiceManager::getPlayerInformation()\n%s", result.c_str()));

	return result;
}

//-----------------------------------------------------------------
void CustomerServiceManager::cancelTickets()
{
	typedef std::vector<unsigned int> CancelTicketList;
	CancelTicketList cancelTicketList;

	// Get the list of tickets to cancel

	CustomerServiceManager::TicketList::const_iterator iterTicketList = s_ticketList.begin();

	for (; iterTicketList != s_ticketList.end(); ++iterTicketList)
	{
		CustomerServiceTicket const &ticket = (*iterTicketList);

		cancelTicketList.push_back(ticket.getTicketId());
	}

	// Cancel all the tickets

	CancelTicketList::const_iterator iterCancelTicketList = cancelTicketList.begin();

	for (; iterCancelTicketList != cancelTicketList.end(); ++iterCancelTicketList)
	{
		unsigned int ticketId = (*iterCancelTicketList);

		requestTicketCancellation(ticketId, false);
	}

	Transceivers::requestTicketsResponse.emitMessage(true);
}

// ============================================================================
