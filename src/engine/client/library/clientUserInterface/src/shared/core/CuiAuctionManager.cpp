//======================================================================

//
// CuiAuctionManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiAuctionManager.h"

#include "clientGame/AuctionManagerClient.h"
#include "clientGame/AuctionManagerClientData.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/IntangibleObject.h"
#include "clientUserInterface/CuiMediator.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIdsAuction.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/IsVendorOwnerMessage.h"
#include "sharedNetworkMessages/IsVendorOwnerResponseMessage.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/NetworkIdManager.h"

//======================================================================

namespace
{
	CuiMessageBox * s_acceptBidBox          = 0;
	CuiMessageBox * s_rejectBox             = 0;
	CuiMessageBox * s_confirmWithdrawBox    = 0;
	CuiMessageBox * s_confirmRejectBox      = 0;
	CuiMessageBox * s_confirmAcceptBidBox   = 0;
	CuiMessageBox * s_sellBox               = 0;
	CuiMessageBox * s_waitingVendorIdBox    = 0;
	CuiMessageBox * s_waitingAuctionIdBox   = 0;

	NetworkId       s_confirmingWithdraw;
	NetworkId       s_confirmingReject;
	NetworkId       s_confirmingAcceptBid;

	NetworkId       s_waitingAcceptId;
	NetworkId       s_waitingRejectId;
	NetworkId       s_waitingSellId;
	NetworkId       s_waitingVendorId;
	NetworkId       s_waitingAuctionId;

	std::string     s_vendorMediatorName;
	std::string     s_auctionMediatorName;

	std::string     s_auctionMarketFullName;
	std::string     s_auctionMarketRegionName;
	std::string     s_vendorMarketFullName;
	std::string     s_vendorMarketRegionName;

	typedef stdset<NetworkId>::fwd NetworkIdSet;
	NetworkIdSet s_waitingRetrieveIds;
	NetworkIdSet s_waitingWithdrawIds;
	
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const CuiAuctionManager::Messages::Retrieved::Payload &,                    CuiAuctionManager::Messages::Retrieved > 
			retrieved;
		MessageDispatch::Transceiver<const CuiAuctionManager::Messages::Withdrawn::Payload &,                    CuiAuctionManager::Messages::Withdrawn > 
			withdrawn;
	}

	class MyCallback : public MessageDispatch::Callback
	{
	public:

		MyCallback () :
		MessageDispatch::Callback ()
		{
			connect (*this, &MyCallback::onItemRetrieved,         static_cast<AuctionManagerClient::Messages::Retrieved *>(0));
			connect (*this, &MyCallback::onAcceptBid,             static_cast<AuctionManagerClient::Messages::AcceptBidResponse *>(0));
			connect (*this, &MyCallback::onWithdraw,              static_cast<AuctionManagerClient::Messages::CancelLiveAuctionResponse *>(0));
			connect (*this, &MyCallback::onCreateAuctionResponse, static_cast<AuctionManagerClient::Messages::CreateAuctionResponse *>(0));
			connect (*this, &MyCallback::onVendorOwnerResponse,   static_cast<AuctionManagerClient::Messages::VendorOwnerResponse *>(0));
		}

		void onCreateAuctionResponse (const AuctionManagerClient::Messages::CreateAuctionResponse::Payload & payload)
		{
			CuiAuctionManager::handleOnSell (payload);
		}

		void onItemRetrieved (const AuctionManagerClient::Messages::Retrieved::Payload & payload)
		{
			CuiAuctionManager::handleOnItemRetrieved (payload);
		}

		void onAcceptBid     (const AuctionManagerClient::Messages::AcceptBidResponse::Payload & payload)
		{
			CuiAuctionManager::handleOnAcceptBid (payload);
		}

		void onWithdraw (const AuctionManagerClient::Messages::CancelLiveAuctionResponse::Payload & payload)
		{
			CuiAuctionManager::handleOnWithdraw (payload);
		}

		void onReject (const AuctionManagerClient::Messages::CancelLiveAuctionResponse::Payload & payload)
		{
			CuiAuctionManager::handleOnReject (payload);
		}

		void onVendorOwnerResponse (const AuctionManagerClient::Messages::VendorOwnerResponse::Payload & payload)
		{
			if (s_waitingVendorId == payload.first)
				CuiAuctionManager::handleOnStartVendor    (payload.first, payload.second.first.first, payload.second.first.second, payload.second.second);
			else if (s_waitingAuctionId == payload.first)
				CuiAuctionManager::handleOnStartAuction   (payload.first, payload.second.first.second, payload.second.second);
		}

		void onMessageBoxClosed (const CuiMessageBox & box)
		{
			if (&box == s_acceptBidBox)
			{
				s_waitingAcceptId = NetworkId::cms_invalid;
				s_acceptBidBox   = 0;
			}
			else if (&box == s_rejectBox)
			{
				s_waitingRejectId = NetworkId::cms_invalid;
				s_rejectBox = 0;
			}
			else if (&box == s_sellBox)
			{
				s_waitingSellId = NetworkId::cms_invalid;
				s_sellBox = 0;
			}
			else if (&box == s_confirmAcceptBidBox)
			{
				if (box.completedAffirmative ())
					CuiAuctionManager::handleAcceptBid (s_confirmingAcceptBid, true);
				s_confirmingAcceptBid = NetworkId::cms_invalid;
			}
			else if (&box == s_confirmWithdrawBox)
			{
				if (box.completedAffirmative ())
					CuiAuctionManager::handleWithdraw (s_confirmingWithdraw, true);
				s_confirmingWithdraw = NetworkId::cms_invalid;
			}
			else if (&box == s_confirmRejectBox)
			{
				if (box.completedAffirmative ())
					CuiAuctionManager::handleReject (s_confirmingReject, true);
				s_confirmingReject = NetworkId::cms_invalid;
			}
		}
	};
	
	MyCallback * s_callback = 0;
	
	void onMessageBoxVendorStartWaiting (const CuiMessageBox & box)
	{
		if (s_waitingVendorIdBox == &box)
		{
			s_waitingVendorIdBox = 0;
			
			if (box.getCompletedButtonType () == CuiMessageBox::GBT_Close)
			{
				s_waitingVendorId = NetworkId::cms_invalid;
			}
		}
	}

	void onMessageBoxAuctionStartWaiting (const CuiMessageBox & box)
	{
		if (s_waitingAuctionIdBox == &box)
		{
			s_waitingAuctionIdBox = 0;
			
			if (box.getCompletedButtonType () == CuiMessageBox::GBT_Close)
			{
				s_waitingAuctionId = NetworkId::cms_invalid;
			}
		}
	}

	int s_vendorTimeout = 60 * 60 * 24 * 7; //7 days	
	NetworkId s_lastContainerItemId;
}

//----------------------------------------------------------------------

void CuiAuctionManager::install ()
{
	s_callback = new MyCallback;

	bool debugVendorTimeout = false;
	DebugFlags::registerFlag (debugVendorTimeout,   "ClientUserInterface", "vendorTimeout");

	if (debugVendorTimeout)
		s_vendorTimeout = 30;
}

//----------------------------------------------------------------------

void CuiAuctionManager::remove ()
{
	delete s_callback;
	s_callback = 0;
}

//----------------------------------------------------------------------

void CuiAuctionManager::handleItemRetrieve (const NetworkId & itemId)
{
	const AuctionManagerClientData * const data = AuctionManagerClient::findAuction (itemId);

	if (!data)
	{
		WARNING (true, ("CuiAuctionManager invalid item to retrieve [%s]", itemId.getValueString ().c_str ()));
		return;
	}

	std::string market_planet;
	std::string market_city;
	std::string market_name;
	
	AuctionManagerClient::getMarketLocationStrings (market_planet, market_city, market_name);

	std::string item_planet;
	std::string item_region;
	std::string item_marketName;
	NetworkId   item_marketId;

	if (!AuctionManagerClient::separateLocation (data->header.location, item_planet, item_region, item_marketName, item_marketId))
	{
		WARNING (true, ("CuiAuctionManager bad"));
		return;
	}

	if (market_planet != item_planet || market_city != item_region)
	{
		Unicode::String localizedLocation;
		AuctionManagerClient::localizeLocation (data->header.location, localizedLocation, true, false);
		Unicode::String result;
		CuiStringVariablesManager::process (CuiStringIdsAuction::err_retrieve_not_at_location_prose, Unicode::emptyString, Unicode::emptyString, localizedLocation, result);
		CuiMessageBox::createInfoBox (result);
		return;
	}

	s_waitingRetrieveIds.insert (itemId);

	AuctionManagerClient::retrieve (itemId, item_marketId);
}

//----------------------------------------------------------------------

void CuiAuctionManager::handleAcceptBid       (const NetworkId & itemId, bool confirmed)
{
#if 1
	UNREF (itemId);
	UNREF (confirmed);

	//-- temp fix for the commodities market accept bid exploit
	CuiMessageBox::createInfoBox (Unicode::narrowToWide ("Temporarily disabled. Please wait for the auction to expire."));
#else
	if (confirmed)
	{
		AuctionManagerClient::acceptBid (itemId);
		
		if (s_acceptBidBox)
			s_acceptBidBox->closeMessageBox ();
		
		s_waitingAcceptId  = itemId;

		s_acceptBidBox = CuiMessageBox::createMessageBox (CuiStringIdsAuction::waiting_accept_bid.localize ());
		NOT_NULL (s_acceptBidBox);
		s_acceptBidBox->setRunner (true);
		
		s_callback->connect (s_acceptBidBox->getTransceiverClosed (), *s_callback, &MyCallback::onMessageBoxClosed);

		return;
	}

	s_confirmingAcceptBid    = itemId;
	s_confirmingWithdraw     = NetworkId::cms_invalid;
	s_confirmingReject       = NetworkId::cms_invalid;

	s_confirmAcceptBidBox = CuiMessageBox::createYesNoBox (CuiStringIdsAuction::confirm_accept.localize ());
	s_callback->connect (s_confirmAcceptBidBox->getTransceiverClosed (), *s_callback, &MyCallback::onMessageBoxClosed);
#endif
}

//----------------------------------------------------------------------

void CuiAuctionManager::handleWithdraw        (const NetworkId & itemId, bool)
{
	s_waitingWithdrawIds.insert (itemId);
	AuctionManagerClient::withdrawItem (itemId);
}

//----------------------------------------------------------------------

void CuiAuctionManager::handleGenericSell    (const NetworkId & itemId, int price, int seconds, bool instant, const Unicode::String & desc, const StringId & sid, bool isPremium, bool isVendorStockroomTransfer)
{
	const ClientObject * const clientObject = safe_cast<const ClientObject *>(NetworkIdManager::getObjectById (itemId));
	if (clientObject)
	{
		const IntangibleObject * const intangible = dynamic_cast<const IntangibleObject *>(clientObject);

		if (intangible)
		{
			if (intangible->getGameObjectType () != SharedObjectTemplate::GOT_data_manufacturing_schematic)
			{
				CuiMessageBox::createInfoBox (CuiStringIdsAuction::err_sell_cannot_sell_intangible.localize ());
				return;
			}
		}
	}

	Unicode::String itemLocalizedName;
	
	// if relisting the item from stock room, see if it's an older
	// item that is stored in the commodities system with a string
	// id as the item name, and if yes, replace the item name with
	// the localized name
	if (isVendorStockroomTransfer)
	{
		AuctionManagerClientData const * const data = AuctionManagerClient::findAuction(itemId);
		if (data)
		{
			// split the auction item name string into its localized components
			std::vector<Unicode::String> undecodedComponents;
			std::vector<Unicode::String> decodedComponents;

			// if any of the component got localized, then reconstruct
			// the auction item name using the localized name and send
			// it back to the commodities server so the item will get
			// relisted with its localized name
			if (0 < data->decodeAuctionItemNameString(undecodedComponents, decodedComponents))
			{
				for (std::vector<Unicode::String>::const_iterator iter = decodedComponents.begin(); iter != decodedComponents.end(); ++iter)
				{
					if (itemLocalizedName.size() > 0)
						itemLocalizedName += uint16(0);

					itemLocalizedName += *iter;
				}
			}
		}
	}
	else if (clientObject)
	{
		itemLocalizedName = clientObject->getLocalizedEnglishName();
	}

	AuctionManagerClient::sellItem (AuctionManagerClient::getMarketObjectId (), itemId, itemLocalizedName, price, instant, seconds, desc, isPremium, isVendorStockroomTransfer);
		
	if (s_sellBox)
		s_sellBox->closeMessageBox ();

	s_waitingSellId = itemId;

	s_sellBox = CuiMessageBox::createMessageBox (sid.localize ());
	s_sellBox->setRunner (true);
	s_callback->connect (s_sellBox->getTransceiverClosed (), *s_callback, &MyCallback::onMessageBoxClosed);
	
	return;
}

//----------------------------------------------------------------------

void CuiAuctionManager::handleAuctionSale     (const NetworkId & itemId, int price, int seconds, bool instant, const Unicode::String & desc, bool confirmed, bool isPremium)
{
	if (confirmed)
	{
		handleGenericSell (itemId, price, seconds, instant, desc, CuiStringIdsAuction::waiting_sell, isPremium, false);
		return;
	}
}

//----------------------------------------------------------------------

void CuiAuctionManager::handleVendorSale      (const NetworkId & itemId, int price, const Unicode::String & desc, bool confirmed, bool isPremium, bool isVendorStockroomTransfer)
{
	if (confirmed)
	{
		handleGenericSell (itemId, price, s_vendorTimeout, true, desc, CuiStringIdsAuction::waiting_vendor_sell, isPremium, isVendorStockroomTransfer);
		return;
	}
}

//----------------------------------------------------------------------

void CuiAuctionManager::handleVendorRelist    (const std::vector<NetworkId> & itemIds)
{	
	if (itemIds.empty())
		return;

	AuctionManagerClient::relistItemsFromStock(itemIds, s_vendorTimeout);
}

//----------------------------------------------------------------------

void CuiAuctionManager::handleReject       (const NetworkId & itemId, bool confirmed)
{
	if (confirmed)
	{
		AuctionManagerClient::withdrawItem (itemId);
		
		if (s_rejectBox)
			s_rejectBox->closeMessageBox ();
		
		s_waitingRejectId = itemId;

		/*
		s_rejectBox = CuiMessageBox::createMessageBox (CuiStringIdsAuction::waiting_reject.localize ());
		NOT_NULL (s_rejectBox);
		s_rejectBox->setRunner (true);

		s_callback->connect (s_rejectBox->getTransceiverClosed (), *s_callback, &MyCallback::onMessageBoxClosed);
*/
		return;
	}
	
	s_confirmingReject      = itemId;
	s_confirmingAcceptBid   = NetworkId::cms_invalid;
	s_confirmingWithdraw    = NetworkId::cms_invalid;
	
	s_confirmRejectBox = CuiMessageBox::createYesNoBox (CuiStringIdsAuction::confirm_reject.localize ());
	s_callback->connect (s_confirmRejectBox->getTransceiverClosed (), *s_callback, &MyCallback::onMessageBoxClosed);
}

//----------------------------------------------------------------------

bool CuiAuctionManager::isWaitingForRetrieve (const NetworkId & id)
{
	return s_waitingRetrieveIds.find (id) != s_waitingRetrieveIds.end ();
}

//----------------------------------------------------------------------

int CuiAuctionManager::getOutstandingRetrieveRequestCount ()
{
	return s_waitingRetrieveIds.size ();
}

//----------------------------------------------------------------------

bool CuiAuctionManager::isWaitingForWithdraw  (const NetworkId & id)
{
	return s_waitingWithdrawIds.find (id) != s_waitingWithdrawIds.end ();
}

//----------------------------------------------------------------------

int CuiAuctionManager::getOutstandingWithdrawRequestCount ()
{
	return s_waitingWithdrawIds.size ();
}

//----------------------------------------------------------------------

void CuiAuctionManager::handleOnItemRetrieved (const AuctionManagerClient::Messages::Retrieved::Payload & payload)
{
	if (!isWaitingForRetrieve (payload.first))
		return;

	if (payload.second.first == ar_OK || payload.second.first == ar_ITEM_NOLONGER_EXISTS || payload.second.first == ar_INVALID_ITEM_REIMBURSAL)
		CuiSystemMessageManager::sendFakeSystemMessage (payload.second.second);
	else
		CuiMessageBox::createInfoBox (payload.second.second);

	s_waitingRetrieveIds.erase (payload.first);

	Transceivers::retrieved.emitMessage (payload);
}

//----------------------------------------------------------------------

void CuiAuctionManager::handleOnAcceptBid (const AuctionManagerClient::Messages::AcceptBidResponse::Payload & payload)
{
	if (s_waitingAcceptId != payload.first)
		return;

	if (s_acceptBidBox)
		s_acceptBidBox->closeMessageBox ();

	if (payload.second.first == ar_OK)
		CuiSystemMessageManager::sendFakeSystemMessage (payload.second.second);
	else
		CuiMessageBox::createInfoBox (payload.second.second);
}

//----------------------------------------------------------------------

void CuiAuctionManager::handleOnWithdraw (const AuctionManagerClient::Messages::CancelLiveAuctionResponse::Payload & payload)
{
	if (!isWaitingForWithdraw (payload.first))
		return;

	if (payload.second.first == ar_OK)
		CuiSystemMessageManager::sendFakeSystemMessage (payload.second.second);
	else
		CuiMessageBox::createInfoBox (payload.second.second);

	s_waitingWithdrawIds.erase (payload.first);
	Transceivers::withdrawn.emitMessage (payload);
}

//----------------------------------------------------------------------

void CuiAuctionManager::handleOnReject (const AuctionManagerClient::Messages::CancelLiveAuctionResponse::Payload & payload)
{
	if (s_waitingRejectId != payload.first)
		return;

	if (s_rejectBox)
		s_rejectBox->closeMessageBox ();

	if (payload.second.first == ar_OK)
		CuiSystemMessageManager::sendFakeSystemMessage (payload.second.second);
	else
		CuiMessageBox::createInfoBox (payload.second.second);
}

//----------------------------------------------------------------------

void CuiAuctionManager::handleOnSell          (const std::pair<NetworkId, ResultInfo> & payload)
{
	if (s_waitingSellId != payload.first)
		return;

	if (s_sellBox)
		s_sellBox->closeMessageBox ();

	if (payload.second.first == ar_OK)
		CuiSystemMessageManager::sendFakeSystemMessage (payload.second.second);
	else
		CuiMessageBox::createInfoBox (payload.second.second);
}

//----------------------------------------------------------------------

void CuiAuctionManager::handleStartVendor     (const NetworkId & vendorId, const std::string & mediatorName)
{
	AuctionManagerClient::reset ();
	if (!s_auctionMediatorName.empty ())
	{	
		CuiMediator * const mediator = CuiMediatorFactory::get (s_auctionMediatorName.c_str (), false);
		if (mediator)
		{
			mediator->close ();
			mediator->closeNextFrame ();
		}
	}
	
	CuiMediator * const mediator2 = CuiMediatorFactory::get (mediatorName.c_str (), false);
	if (mediator2)
	{
		mediator2->close ();
		mediator2->closeNextFrame ();
	}
	
	s_waitingVendorId = vendorId;
	s_vendorMediatorName = mediatorName;

	if (s_waitingVendorIdBox)
		s_waitingVendorIdBox->closeMessageBox ();

	if (s_waitingAuctionIdBox)
		s_waitingAuctionIdBox->closeMessageBox ();

	s_waitingVendorIdBox = CuiMessageBox::createMessageBox (CuiStringIdsAuction::waiting_vendor_owner.localize (), onMessageBoxVendorStartWaiting);
	s_waitingVendorIdBox->setRunner (true);

	if (Game::getSinglePlayer ())
	{
		static int owner = 0;
		handleOnStartVendor (vendorId, owner, ar_OK, std::string ("planet.region.name.id.location"));

		if (++owner > IsVendorOwnerResponseMessage::vor_HasNoOwner)
			owner = 0;
	}
	else
	{
		const IsVendorOwnerMessage msg (vendorId);		
		GameNetwork::send (msg, true);
	}
}

//----------------------------------------------------------------------

void CuiAuctionManager::handleOnStartVendor   (const NetworkId & vendorId, int ownerCode, int resultCode, const std::string & marketFullName)
{
	if (s_waitingVendorId != vendorId)
		return;
	
	if (s_waitingVendorIdBox)
		s_waitingVendorIdBox->closeMessageBox ();

	if (s_waitingAuctionIdBox)
		s_waitingAuctionIdBox->closeMessageBox ();

	if (resultCode != ar_OK)
	{
		CuiMessageBox::createInfoBox (CuiStringIdsAuction::err_vendor_terminal_error.localize ());
		return;
	}

	if (ownerCode == IsVendorOwnerResponseMessage::vor_HasNoOwner)
	{
		CuiMessageBox::createInfoBox (CuiStringIdsAuction::err_vendor_terminal_no_owner.localize ());
		return;
	}

	std::string planet;
	std::string name;
	std::string regionName;
	NetworkId dummyId;

	if (!AuctionManagerClient::separateLocation          (marketFullName, planet, regionName, name, dummyId))
	{
		WARNING (true, ("CuiAuctionManager handleOnStartVendor bad location."));
		return;
	}

	AuctionManagerClient::setMarketObjectId   (vendorId);
	AuctionManagerClient::setMarketRegionName (regionName);
	AuctionManagerClient::setIsAtVendor     (true, ownerCode == IsVendorOwnerResponseMessage::vor_IsOwner);

	CuiMediatorFactory::activateInWorkspace (s_vendorMediatorName.c_str ());
}

//----------------------------------------------------------------------

void CuiAuctionManager::handleStartAuction     (const NetworkId & marketId, const std::string & mediatorName)
{
	AuctionManagerClient::reset ();

	if (!s_vendorMediatorName.empty ())
	{	
		CuiMediator * const mediator = CuiMediatorFactory::get (s_vendorMediatorName.c_str (), false);
		if (mediator)
		{
			mediator->close ();
			mediator->closeNextFrame ();
		}
	}
	
	CuiMediator * const mediator2 = CuiMediatorFactory::get (mediatorName.c_str (), false);
	if (mediator2)
	{
		mediator2->close ();
		mediator2->closeNextFrame ();
	}
	
	s_waitingAuctionId    = marketId;
	s_auctionMediatorName = mediatorName;
	
	if (s_waitingAuctionIdBox)
		s_waitingAuctionIdBox->closeMessageBox ();

	if (s_waitingVendorIdBox)
		s_waitingVendorIdBox->closeMessageBox ();
	
	s_waitingAuctionIdBox = CuiMessageBox::createMessageBox (CuiStringIdsAuction::waiting_auction_info.localize (), onMessageBoxAuctionStartWaiting);
	s_waitingAuctionIdBox->setRunner (true);

	if (Game::getSinglePlayer ())
	{
		handleOnStartAuction (marketId, ar_OK, std::string ("planet.region.name.id.location"));
	}
	else
	{
		const IsVendorOwnerMessage msg (marketId);		
		GameNetwork::send (msg, true);
	}
}

//----------------------------------------------------------------------

void CuiAuctionManager::handleOnStartAuction   (const NetworkId & marketId, int resultCode, const std::string & marketFullName)
{
	if (s_waitingAuctionId != marketId)
		return;
	
	if (s_waitingAuctionIdBox)
		s_waitingAuctionIdBox->closeMessageBox ();

	if (s_waitingVendorIdBox)
		s_waitingVendorIdBox->closeMessageBox ();

	if (resultCode != ar_OK)
	{
		CuiMessageBox::createInfoBox (CuiStringIdsAuction::err_auction_terminal_error.localize ());
		return;
	}

	std::string planet;
	std::string name;
	std::string regionName;
	NetworkId dummyId;

	if (!AuctionManagerClient::separateLocation          (marketFullName, planet, regionName, name, dummyId))
	{
		WARNING (true, ("CuiAuctionManager handleOnStartAuction bad location."));
		return;
	}
	
	AuctionManagerClient::setMarketObjectId (marketId);
	AuctionManagerClient::setMarketRegionName (regionName);
	AuctionManagerClient::setIsAtVendor     (false, false);

	CuiMediatorFactory::activateInWorkspace (s_auctionMediatorName.c_str ());
}

//----------------------------------------------------------------------

void CuiAuctionManager::setLastContainerItemId(NetworkId const &itemId)
{
	s_lastContainerItemId = itemId;
}

//----------------------------------------------------------------------

NetworkId const &CuiAuctionManager::getLastContainerItemId()
{
	return s_lastContainerItemId;
}

//======================================================================
