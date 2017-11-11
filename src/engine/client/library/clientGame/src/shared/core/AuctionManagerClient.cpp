//======================================================================
//
// AuctionManagerClient.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/AuctionManagerClient.h"

#include "clientGame/AuctionManagerClientData.h"
#include "clientGame/AuctionManagerClientHistory.h"
#include "clientGame/AuctionManagerClientListener.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientGame/TangibleObject.h"
#include "clientUserInterface/CuiAuctionManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIdsAuction.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/AsynchronousLoader.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Watcher.h"
#include "sharedGame/AuctionManager.h"
#include "sharedGame/TextManager.h"
#include "sharedMath/Vector.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/AcceptAuctionMessage.h"
#include "sharedNetworkMessages/AuctionErrorCodes.h"
#include "sharedNetworkMessages/BidAuctionMessage.h"
#include "sharedNetworkMessages/CancelLiveAuctionMessage.h"
#include "sharedNetworkMessages/CreateAuctionMessage.h"
#include "sharedNetworkMessages/CreateImmediateAuctionMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/GetAuctionDetails.h"
#include "sharedNetworkMessages/RetrieveAuctionItemMessage.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedRandom/Random.h"
#include "unicodeArchive/UnicodeArchive.h"

#include <map>
#include <vector>

//======================================================================

namespace
{
	typedef AuctionManagerClient::DataVector          DataVector;
	typedef AuctionManagerClient::AuctionHeaderVector AuctionHeaderVector;

	float s_timeSinceDecrementAuctionTimes;

	DataVector            s_dataVectors     [AuctionManagerClient::T_numTypes];
	int                   s_currentIndices  [AuctionManagerClient::T_numTypes];
	int                   s_pageSizes       [AuctionManagerClient::T_numTypes];
	bool                  s_hasMoreAuctions [AuctionManagerClient::T_numTypes];

	typedef stdmap<NetworkId, Auction::ItemDataDetails>::fwd DetailsMap;
	DetailsMap            s_details;

	namespace Transceivers
	{
		MessageDispatch::Transceiver<const AuctionManagerClient::Messages::ListAllChanged::Payload &,               AuctionManagerClient::Messages::ListAllChanged >
			listAllChanged;
		MessageDispatch::Transceiver<const AuctionManagerClient::Messages::ListSalesChanged::Payload &,             AuctionManagerClient::Messages::ListSalesChanged >
			listSalesChanged;
		MessageDispatch::Transceiver<const AuctionManagerClient::Messages::ListBidsChanged::Payload &,              AuctionManagerClient::Messages::ListBidsChanged >
			listBidsChanged;
		MessageDispatch::Transceiver<const AuctionManagerClient::Messages::ListAvailableChanged::Payload &,         AuctionManagerClient::Messages::ListAvailableChanged >
			listAvailableChanged;
		MessageDispatch::Transceiver<const AuctionManagerClient::Messages::ListVendorSellingChanged::Payload &,     AuctionManagerClient::Messages::ListVendorSellingChanged >
			listVendorSellingChanged;
		MessageDispatch::Transceiver<const AuctionManagerClient::Messages::ListVendorOffersChanged::Payload &,      AuctionManagerClient::Messages::ListVendorOffersChanged >
			listVendorOffersChanged;
		MessageDispatch::Transceiver<const AuctionManagerClient::Messages::ListVendorStockroomChanged::Payload &,   AuctionManagerClient::Messages::ListVendorStockroomChanged >
			listVendorStockroomChanged;
		MessageDispatch::Transceiver<const AuctionManagerClient::Messages::ListVendorLocationChanged::Payload &,    AuctionManagerClient::Messages::ListVendorLocationChanged >
			listVendorLocationChanged;
		MessageDispatch::Transceiver<const AuctionManagerClient::Messages::DetailsReceived::Payload &,              AuctionManagerClient::Messages::DetailsReceived >
			detailsReceived;
		MessageDispatch::Transceiver<const AuctionManagerClient::Messages::CreateAuctionResponse::Payload &,        AuctionManagerClient::Messages::CreateAuctionResponse >
			createAuctionResponse;
		MessageDispatch::Transceiver<const AuctionManagerClient::Messages::BidResponse::Payload &,                  AuctionManagerClient::Messages::BidResponse >
			bidResponse;
		MessageDispatch::Transceiver<const AuctionManagerClient::Messages::CancelLiveAuctionResponse::Payload &,    AuctionManagerClient::Messages::CancelLiveAuctionResponse >
			cancelLiveAuctionResponse;
		MessageDispatch::Transceiver<const AuctionManagerClient::Messages::AuctionToViewChanged::Payload &,         AuctionManagerClient::Messages::AuctionToViewChanged >
			auctionToViewChanged;
		MessageDispatch::Transceiver<const AuctionManagerClient::Messages::AuctionToBidOnChanged::Payload &,        AuctionManagerClient::Messages::AuctionToBidOnChanged >
			auctionToBidOnChanged;
		MessageDispatch::Transceiver<const AuctionManagerClient::Messages::TraverseAuctionToView::Payload &,        AuctionManagerClient::Messages::TraverseAuctionToView >
			traverseAuctionToView;
		MessageDispatch::Transceiver<const AuctionManagerClient::Messages::Retrieved::Payload &,                    AuctionManagerClient::Messages::Retrieved >
			retrieved;
		MessageDispatch::Transceiver<const AuctionManagerClient::Messages::RetrieveRequested::Payload &,            AuctionManagerClient::Messages::RetrieveRequested >
			retrieveRequested;
		MessageDispatch::Transceiver<const AuctionManagerClient::Messages::WithdrawRequested::Payload &,            AuctionManagerClient::Messages::WithdrawRequested >
			withdrawRequested;
		MessageDispatch::Transceiver<const AuctionManagerClient::Messages::AcceptBidResponse::Payload &,            AuctionManagerClient::Messages::AcceptBidResponse >
			acceptBid;
		MessageDispatch::Transceiver<const AuctionManagerClient::Messages::AuctionToSetPriceOnChanged::Payload &,   AuctionManagerClient::Messages::AuctionToSetPriceOnChanged >
			auctionToSetPriceOnChanged;
		MessageDispatch::Transceiver<const AuctionManagerClient::Messages::VendorOwnerResponse::Payload &,          AuctionManagerClient::Messages::VendorOwnerResponse >
			vendorOwnerResponse;
		MessageDispatch::Transceiver<const AuctionManagerClient::Messages::ItemTypeListChanged::Payload &,          AuctionManagerClient::Messages::ItemTypeListChanged >
			itemTypeListChanged;
		MessageDispatch::Transceiver<const AuctionManagerClient::Messages::ResourceTypeListChanged::Payload &,      AuctionManagerClient::Messages::ResourceTypeListChanged >
			resourceTypeListChanged;
	}

	//----------------------------------------------------------------------

	void simulateHeaders (int type, AuctionHeaderVector & ahv, bool selfOwner, bool zeroTimer, int startingIndex)
	{
		const DataVector & dv = s_dataVectors [type];

		const Object * const player = Game::getPlayer ();
		NetworkId selfId;
		if (player)
			selfId = player->getNetworkId ();

		for (DataVector::const_iterator it = dv.begin (); it != dv.end ();)
		{
			const AuctionManagerClientData & d = *it;
			if (Random::random (0, 3) == 0)
			{
				continue;
			}
			else if ((d.header.buyNowPrice <= 0) && Random::random (0, 5) == 0)
			{
				AuctionManagerClient::Header header = d.header;
				//header.highBid += Random::random (1, 100) * 10;
				if (zeroTimer)
					header.timer    = 0;
				if (selfOwner)
					header.ownerId  = selfId;

				ahv.push_back (header);
			}

			++it;
		}

		const int numNew = Random::random (0, 5);

		for (int i = 0; i < numNew; ++i)
		{
			ahv.push_back (AuctionManagerClientData::constructSimulationHeader ());

			if (zeroTimer)
				ahv.back ().timer    = 0;

			if (selfOwner)
				ahv.back ().ownerId = selfId;
		}

		s_currentIndices  [type]   = startingIndex;
		s_hasMoreAuctions [type]   = Random::random (0, 2) != 0;
	}

	//----------------------------------------------------------------------

	void decrementTimeouts (DataVector & dv, int timeToDecrement)
	{
		for (DataVector::iterator it = dv.begin (); it != dv.end (); ++it)
		{
			AuctionManagerClientData & d = *it;
			if (timeToDecrement < d.header.timer)
				d.header.timer -= timeToDecrement;
			else
				d.header.timer = 0;
		}
	}

	//----------------------------------------------------------------------

	typedef Watcher<ClientObject> ObjectWatcher;
	typedef stdmap<NetworkId, ObjectWatcher>::fwd ObjectMap;
	ObjectMap s_objects;

	std::string ms_marketRegionName;

	// this must be kept in sync in the following files
	// src/engine/client/library/clientGame/src/shared/core/AuctionManagerClient.cpp (s_maxBid)
	// src/engine/server/application/CommoditiesServer/src/shared/Auction.cpp (MAX_BID)
	// src/engine/server/library/serverGame/src/shared/commoditiesMarket/CommoditiesMarket.cpp (MAX_BID)
	const int s_maxBid = 10000000;
	const int s_maxVendorPrice = 833333333;

	const uint8 s_minTextFilterTokenLength = 1;

	AuctionManagerClient::ItemTypeList s_itemTypeList;
	AuctionManagerClient::ItemTypeListServer s_itemTypeListServer;
	std::string s_itemTypeListVersion;

	AuctionManagerClient::ResourceTypeList s_resourceTypeList;
	std::string s_resourceTypeListVersion;
}

//----------------------------------------------------------------------

CachedNetworkId   AuctionManagerClient::ms_marketObjectId;
NetworkId         AuctionManagerClient::ms_auctionToBidOn;
NetworkId         AuctionManagerClient::ms_auctionToView;
NetworkId         AuctionManagerClient::ms_auctionToSetPriceOn;
std::vector<NetworkId> AuctionManagerClient::ms_itemsToSellFromStock;
bool              AuctionManagerClient::ms_atCommodityMarket;
bool              AuctionManagerClient::ms_atVendor;
bool              AuctionManagerClient::ms_playerOwnsVendor;
bool              AuctionManagerClient::s_listRequestOutstanding [AuctionManagerClient::T_numTypes];
bool              AuctionManagerClient::s_listValid              [AuctionManagerClient::T_numTypes];

//----------------------------------------------------------------------

void AuctionManagerClient::install ()
{
	InstallTimer const installTimer("AuctionManagerClient::install");

	AuctionManagerClientListener::install ();
	reset ();
}

//----------------------------------------------------------------------

void AuctionManagerClient::remove ()
{
	reset ();
	AuctionManagerClientListener::remove ();
}

//----------------------------------------------------------------------

void AuctionManagerClient::reset ()
{
	for (ObjectMap::iterator it = s_objects.begin (); it != s_objects.end (); ++it)
	{
		ClientObject * const obj = (*it).second.getPointer ();
		delete obj;
	}

	for (int i = 0; i < T_numTypes; ++i)
	{
		s_listRequestOutstanding [i] = false;
		s_listValid              [i] = false;
		s_dataVectors [i].clear ();
		s_currentIndices   [i] = 0;
		s_pageSizes        [i] = 0;
		s_hasMoreAuctions  [i] = false;
	}

	s_details.clear ();
	s_objects.clear ();

	ms_atCommodityMarket  = false;
	ms_atVendor           = false;
	ms_playerOwnsVendor   = false;

	ms_marketObjectId      = NetworkId::cms_invalid;
	ms_auctionToBidOn      = NetworkId::cms_invalid;
	ms_auctionToView       = NetworkId::cms_invalid;
	ms_auctionToSetPriceOn = NetworkId::cms_invalid;
}

//----------------------------------------------------------------------

AuctionManagerClient::Data *  AuctionManagerClient::findAuction      (DataVector & dv, const NetworkId & itemId)
{
	for (DataVector::iterator it = dv.begin (); it != dv.end (); ++it)
	{
		Data & data = *it;
		if (data.header.itemId == itemId)
			return &data;
	}

	return 0;
}

//----------------------------------------------------------------------

AuctionManagerClient::Data * AuctionManagerClient::findAuction  (const NetworkId & itemId)
{
	for (int i = 0; i < T_numTypes; ++i)
	{
		DataVector & dv = s_dataVectors [i];

		Data * const data = findAuction (dv, itemId);
		if (data)
			return data;
	}

	return 0;
}

//----------------------------------------------------------------------

bool  AuctionManagerClient::removeAuction      (int type, const NetworkId & itemId)
{
	DataVector & dv = s_dataVectors [type];
	for (DataVector::iterator it = dv.begin (); it != dv.end (); ++it)
	{
		Data & data = *it;
		if (data.header.itemId == itemId)
		{
			dv.erase (it);
			fireListChanged (type);
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------

bool AuctionManagerClient::removeAuction  (const NetworkId & itemId)
{
	bool retval = false;

	for (int i = 0; i < T_numTypes; ++i)
		retval = removeAuction (i, itemId) || retval;

	return retval;
}

//----------------------------------------------------------------------

void AuctionManagerClient::addAuction                (int type, const Data & data)
{

	Data * const olddata = findAuction (s_dataVectors [type], data.header.itemId);
	if (olddata)
		*olddata = data;
	else
		s_dataVectors [type].push_back (data);

	fireListChanged (type);
}

//----------------------------------------------------------------------

void AuctionManagerClient::fireListChanged (int type)
{
	switch (type)
	{
	case T_all:
		Transceivers::listAllChanged.emitMessage (0);
		break;
	case T_mySales:
		Transceivers::listSalesChanged.emitMessage (0);
		break;
	case T_myBids:
		Transceivers::listBidsChanged.emitMessage (0);
		break;
	case T_available:
		Transceivers::listAvailableChanged.emitMessage (0);
		break;
	case T_vendorSelling:
		Transceivers::listVendorSellingChanged.emitMessage (0);
		break;
	case T_vendorOffers:
		Transceivers::listVendorOffersChanged.emitMessage (0);
		break;
	case T_vendorStockroom:
		Transceivers::listVendorStockroomChanged.emitMessage (0);
		break;
	case T_location:
		Transceivers::listVendorLocationChanged.emitMessage (0);
		break;
	}
}

//----------------------------------------------------------------------

bool AuctionManagerClient::isInStockroom             (const NetworkId & itemId)
{
	return findAuction (s_dataVectors [T_vendorStockroom], itemId) != 0;
}

//----------------------------------------------------------------------

bool AuctionManagerClient::isItemRetrievable         (const NetworkId & itemId)
{
	const Object * const player = Game::getPlayer ();
	if (!player)
		return false;

	const Data * const data = findAuction (itemId);
	if (data)
	{
		const NetworkId & playerId = player->getNetworkId ();
		if (data->header.ownerId ==  playerId || data->header.highBidderId == playerId)
		{
			if (!data->header.isActive ())
				return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------

bool AuctionManagerClient::isItemWithdrawable        (const NetworkId & itemId)
{
	const Object * const player = Game::getPlayer ();
	if (!player)
		return false;

	const Data * const data = findAuction (itemId);
	if (data)
	{
		if (data->header.ownerId == player->getNetworkId () && data->header.isActive ())
			return true;
	}

	return false;
}

//----------------------------------------------------------------------

bool AuctionManagerClient::isItemAcceptable          (const NetworkId & itemId)
{
	const Object * const player = Game::getPlayer ();
	if (!player)
		return false;

	const NetworkId & playerId = player->getNetworkId ();

	const Data * const data = findAuction (itemId);
	if (data)
	{
		return (data->header.ownerId == playerId && data->header.highBidderId.isValid () && data->header.highBidderId != playerId && data->header.isActive ());
	}

	return false;
}

//----------------------------------------------------------------------

void AuctionManagerClient::pruneObjects ()
{
	for (ObjectMap::iterator it = s_objects.begin (); it != s_objects.end (); )
	{
		const NetworkId & id = (*it).first;

		if (!findAuction (id))
		{
			ClientObject * const obj = (*it).second.getPointer ();
			delete obj;
			s_objects.erase (it++);
		}
		else
			++it;
	}
}

//----------------------------------------------------------------------

ClientObject * AuctionManagerClient::getClientObjectForAuction (const NetworkId & itemId)
{
	const ObjectMap::iterator it = s_objects.find (itemId);

	if (it != s_objects.end ())
		return (*it).second;

	Data * const data = findAuction (itemId);

	if (data && data->hasDetails)
	{
		const bool asynchronousLoaderEnabled = AsynchronousLoader::isEnabled ();
		AsynchronousLoader::disable ();

		ClientObject * const obj = safe_cast<ClientObject *>(ObjectTemplate::createObject (data->details.templateName.c_str ()));

		if (asynchronousLoaderEnabled)
			AsynchronousLoader::enable ();

		if (!obj)
			WARNING (true, ("Could not create auction object for [%s]", data->details.templateName.c_str ()));
		else
		{
			Unicode::String objectName;
			data->constructLocalizedName (objectName);
			obj->setObjectName (objectName);

			if (!data->details.appearanceString.empty ())
			{
				TangibleObject * const tangible = dynamic_cast<TangibleObject *>(obj);
				if (tangible)
				{
					CustomizationData * const cd = tangible->fetchCustomizationData ();

					if (cd)
					{
						cd->loadLocalDataFromString (data->details.appearanceString);
						cd->release ();
					}
				}
			}

			obj->endBaselines ();
		}

		s_objects.insert (std::make_pair (itemId, obj));
		return obj;
	}

	return 0;
}

//----------------------------------------------------------------------

void AuctionManagerClient::receiveHeaders       (const AuctionHeaderVector & ahv, int queryOffset, bool hasMorePages)
{
	handleReceiveHeaders (T_all, ahv, queryOffset, hasMorePages);
}

//----------------------------------------------------------------------

void AuctionManagerClient::receiveHeadersSales       (const AuctionHeaderVector & ahv, int queryOffset, bool hasMorePages)
{
	handleReceiveHeaders (T_mySales, ahv, queryOffset, hasMorePages);
}

//----------------------------------------------------------------------

void AuctionManagerClient::receiveHeadersBids      (const AuctionHeaderVector & ahv, int queryOffset, bool hasMorePages)
{
	handleReceiveHeaders (T_myBids, ahv, queryOffset, hasMorePages);
}

//----------------------------------------------------------------------

void AuctionManagerClient::receiveHeadersAvailable (const AuctionHeaderVector & ahv, int queryOffset, bool hasMorePages)
{
	handleReceiveHeaders (T_available, ahv, queryOffset, hasMorePages);
}

//----------------------------------------------------------------------

void AuctionManagerClient::receiveHeadersVendorSelling         (const AuctionHeaderVector & ahv, int queryOffset, bool hasMorePages)
{
	handleReceiveHeaders (T_vendorSelling, ahv, queryOffset, hasMorePages);
}

//----------------------------------------------------------------------

void AuctionManagerClient::receiveHeadersVendorOffers          (const AuctionHeaderVector & ahv, int queryOffset, bool hasMorePages)
{
	handleReceiveHeaders (T_vendorOffers, ahv, queryOffset, hasMorePages);
}

//----------------------------------------------------------------------

void AuctionManagerClient::receiveHeadersVendorStockroom       (const AuctionHeaderVector & ahv, int queryOffset, bool hasMorePages)
{
	handleReceiveHeaders (T_vendorStockroom, ahv, queryOffset, hasMorePages);
}

//----------------------------------------------------------------------

void AuctionManagerClient::receiveHeadersLocation(const AuctionHeaderVector & ahv, int queryOffset, bool hasMorePages)
{
	handleReceiveHeaders (T_location, ahv, queryOffset, hasMorePages);
}

//----------------------------------------------------------------------

/**
* Update volatile fields like timer, flags, etc...
*/

void AuctionManagerClient::handleReceiveHeaders (int type, const AuctionHeaderVector & ahv, int queryOffset, bool hasMorePages)
{
	//@todo preserve details we have already received

	for (int i = 0; i < T_numTypes; ++i)
	{
		DataVector & dv = s_dataVectors [i];

		if (i == type)
		{
			dv.clear ();
			dv.reserve (ahv.size ());

			if (!Game::getSinglePlayer ())
			{
				s_currentIndices   [i] = queryOffset;
				s_hasMoreAuctions  [i] = hasMorePages;
				if (hasMorePages)
					s_pageSizes [i] = ahv.size ();
			}

			for (AuctionHeaderVector::const_iterator it = ahv.begin (); it != ahv.end (); ++it)
			{
				AuctionManagerClientData data;
				data.setHeader (*it);

				const DetailsMap::const_iterator dmi = s_details.find (data.header.itemId);
				if (dmi != s_details.end ())
					data.setDetails ((*dmi).second);

				dv.push_back (data);
			}

			continue;
		}

		for (AuctionHeaderVector::const_iterator it = ahv.begin (); it != ahv.end (); ++it)
		{
			const Header & incomingDataHeader = *it;

			Data * const data = findAuction (dv, incomingDataHeader.itemId);

			if (data)
			{
				data->header = incomingDataHeader;
			}
		}
	}

	AuctionManagerClient::pruneObjects ();
	s_listRequestOutstanding [type] = false;
	s_listValid [type] = true;
	fireListChanged (type);
}

//----------------------------------------------------------------------

void AuctionManagerClient::receiveDetails       (const Auction::ItemDataDetails & details)
{
	Data * theData = 0;

	s_details [details.itemId] = details;

	for (int i = 0; i < T_numTypes; ++i)
	{
		DataVector & dv = s_dataVectors [i];

		Data * const data = findAuction (dv, details.itemId);
		if (data)
		{
			data->setDetails (details);

			if (Game::isProfanityFiltered())
			{
				data->details.userDescription = TextManager::filterText(data->details.userDescription);
			}

			theData = data;
		}
	}

	if (theData)
		Transceivers::detailsReceived.emitMessage (*theData);
}

//----------------------------------------------------------------------

void AuctionManagerClient::receiveCreateAuctionResponse (const NetworkId & id, int resultCode, const std::string & itemRestrictedRejectionMessage)
{
	s_details.erase (id);

	bool wasAuction = false;
	AuctionManagerClientHistory::saleRemove (id, wasAuction);

	Unicode::String str;

	switch (resultCode)
	{
	case ar_OK:
		if (wasAuction)
			CuiStringIdsAuction::create_auction_successful.localize (str);
		else
			CuiStringIdsAuction::create_sale_successful.localize (str);
		break;
	case ar_INVALID_AUCTIONER:
		if (wasAuction)
			CuiStringIdsAuction::err_create_auction_invalid_auctioner.localize (str);
		else
			CuiStringIdsAuction::err_create_sale_invalid_auctioner.localize (str);
		break;
	case ar_INVALID_ITEM_ID:
		if (wasAuction)
			CuiStringIdsAuction::err_create_auction_invalid_item.localize (str);
		else
			CuiStringIdsAuction::err_create_sale_invalid_item.localize (str);
		break;
	case ar_INVALID_CONTAINER_ID:
		if (wasAuction)
			CuiStringIdsAuction::err_create_auction_invalid_container.localize (str);
		else
			CuiStringIdsAuction::err_create_sale_invalid_container.localize (str);
		break;
	case ar_INVALID_MINIMUM_BID:
		if (wasAuction)
			CuiStringIdsAuction::err_create_auction_invalid_price.localize (str);
		else
			CuiStringIdsAuction::err_create_sale_invalid_price.localize (str);
		break;
	case ar_INVALID_AUCTION_LENGTH:
		if (wasAuction)
			CuiStringIdsAuction::err_create_auction_invalid_length.localize (str);
		else
			CuiStringIdsAuction::err_create_sale_invalid_length.localize (str);
		break;
	case ar_ITEM_ALREADY_AUCTIONED:
		if (wasAuction)
			CuiStringIdsAuction::err_create_auction_already_auctioned.localize (str);
		else
			CuiStringIdsAuction::err_create_sale_already_auctioned.localize (str);
		break;
	case ar_NOT_ITEM_OWNER:
		if (wasAuction)
			CuiStringIdsAuction::err_create_auction_not_owner.localize (str);
		else
			CuiStringIdsAuction::err_create_sale_not_owner.localize (str);
		break;
	case ar_IN_TRADE:
		if (wasAuction)
			CuiStringIdsAuction::err_create_auction_in_trade.localize (str);
		else
			CuiStringIdsAuction::err_create_sale_in_trade.localize (str);
		break;
	case ar_IN_CRATE:
		if (wasAuction)
			CuiStringIdsAuction::err_create_auction_in_crate.localize (str);
		else
			CuiStringIdsAuction::err_create_sale_in_crate.localize (str);
		break;
	case ar_NOT_ALLOWED:
		if (wasAuction)
			CuiStringIdsAuction::err_create_auction_not_allowed.localize (str);
		else
			CuiStringIdsAuction::err_create_sale_not_allowed.localize (str);
		break;
	case ar_NOT_EMPTY:
		if (wasAuction)
			CuiStringIdsAuction::err_create_auction_not_empty.localize (str);
		else
			CuiStringIdsAuction::err_create_sale_not_empty.localize (str);
		break;
	case ar_TOO_MANY_VENDORS:
		CuiStringIdsAuction::err_create_sale_too_many_vendors.localize (str);
		break;
	case ar_TOO_MANY_VENDOR_ITEMS:
		CuiStringIdsAuction::err_create_sale_too_many_vendor_items.localize (str);
		break;
	case ar_NOT_ENOUGH_MONEY:
		{
			//@todo: this should be script driven
			static const int createCost = 20;
			if (wasAuction)
				CuiStringVariablesManager::process (CuiStringIdsAuction::err_create_auction_not_enough_money_prose, Unicode::emptyString, Unicode::emptyString, Unicode::emptyString, createCost, 0.0f, str);
			else
				CuiStringVariablesManager::process (CuiStringIdsAuction::err_create_sale_not_enough_money_prose, Unicode::emptyString, Unicode::emptyString, Unicode::emptyString, createCost, 0.0f, str);
		}
		break;

	case ar_TOO_MANY_AUCTIONS:
		{
			CuiStringIdsAuction::err_too_many_auctions.localize (str);
		}
		break;
	case ar_BID_TOO_HIGH:
		{
			CuiStringVariablesData data;
			data.digit_i = s_maxBid;

			if (wasAuction)
			{
				CuiStringVariablesManager::process(CuiStringIdsAuction::err_create_auction_bid_too_high, data, str);
			}
			else
			{
				CuiStringVariablesManager::process(CuiStringIdsAuction::err_create_sale_bid_too_high, data, str);
			}
		}
		break;
	case ar_PRICE_TOO_HIGH:
		{
			CuiStringVariablesData data;
			data.digit_i = s_maxVendorPrice;

			CuiStringVariablesManager::process(CuiStringIdsAuction::err_create_sale_price_too_high, data, str);
		}
		break;
	case ar_VENDOR_DEACTIVATED:
		{
			CuiStringIdsAuction::err_vendor_deactivated.localize(str);
		}
		break;
	case ar_IS_BIOLINKED:
		if (wasAuction)
			CuiStringIdsAuction::err_create_auction_is_biolinked.localize (str);
		else
			CuiStringIdsAuction::err_create_sale_is_biolinked.localize (str);
		break;
	case ar_ITEM_EQUIPPED:
		if (wasAuction)
			CuiStringIdsAuction::err_create_auction_item_equipped.localize (str);
		else
			CuiStringIdsAuction::err_create_sale_item_equipped.localize (str);
		break;
	case ar_ITEM_RESTRICTED:
		if (itemRestrictedRejectionMessage.empty())
			str = StringId::decodeString(Unicode::narrowToWide("@ui_auc:reject_restricted_item"));
		else
			str = StringId::decodeString(Unicode::narrowToWide(itemRestrictedRejectionMessage));
		break;
	default:
		WARNING (true, ("AuctionManagerClient::receiveCreateAuctionResponse unhandled result %d", resultCode));
		if (wasAuction)
			CuiStringIdsAuction::err_create_auction_unknown.localize (str);
		else
			CuiStringIdsAuction::err_create_sale_unknown.localize (str);
		break;
	}

	Transceivers::createAuctionResponse.emitMessage (Messages::CreateAuctionResponse::Payload (id, Messages::ResultInfo (resultCode, str)));
}

//----------------------------------------------------------------------

void AuctionManagerClient::receiveBidResponse           (const NetworkId & id, int resultCode)
{
	bool wasBuying = false;
	AuctionManagerClientHistory::bidRemove (id, wasBuying);

	const Object * const player = Game::getPlayer ();
	const NetworkId & playerId = player ? player->getNetworkId () : NetworkId::cms_invalid;

	Unicode::String str;

	switch (resultCode)
	{
	case ar_OK:
		if (wasBuying)
		{
			bool playerOwns = false;
			const bool isVendor = isAtVendor (playerOwns);

			const Data * const data = findAuction (id);
			if (!isVendor)
			{
				if (!data)
				{
					CuiStringIdsAuction::buy_successful_local.localize (str);
				}
				else
				{
					std::string planet;
					std::string region;
					std::string name;
					NetworkId   marketId;

					separateLocation (data->header.location, planet, region, name, marketId);

					std::string marketPlanet;
					std::string marketRegion;
					std::string marketName;

					getMarketLocationStrings (marketPlanet, marketRegion, marketName);

					if (marketPlanet == planet && marketRegion == region)
						CuiStringIdsAuction::buy_successful_local.localize (str);
					else if (marketPlanet == planet)
					{
						CuiStringVariablesManager::process (CuiStringIdsAuction::buy_successful_planet_prose, Unicode::emptyString, StringId::decodeString (Unicode::narrowToWide (region)), Unicode::emptyString, str);
					}
					else
					{
						CuiStringVariablesManager::process (CuiStringIdsAuction::buy_successful_galaxy_prose, Unicode::emptyString, StringId::decodeString (Unicode::narrowToWide (region)), StringId ("planet_n", planet).localize (), str);
					}
				}
			}
			else
			{
				if (playerOwns)
				{
					if (!data)
					{
						CuiStringIdsAuction::buy_vendor_owner_successful_local.localize (str);
					}
					else
					{
						std::string planet;
						std::string region;
						std::string name;
						NetworkId   marketId;

						separateLocation (data->header.location, planet, region, name, marketId);

						std::string marketPlanet;
						std::string marketRegion;
						std::string marketName;

						getMarketLocationStrings (marketPlanet, marketRegion, marketName);

						if (marketId == getMarketObjectId ())
							CuiStringIdsAuction::buy_vendor_owner_successful_local.localize (str);
						else
						{
							CuiStringVariablesManager::process (CuiStringIdsAuction::buy_vendor_owner_successful_remote, StringId::decodeString (Unicode::narrowToWide (marketName)), StringId::decodeString (Unicode::narrowToWide (region)), StringId ("planet_n", planet).localize (), str);
						}
					}
				}
				else
				{
					CuiStringIdsAuction::buy_vendor_buyer_successful.localize (str);
				}
			}

			{
				Data oldData;
				bool oldDataSet = false;

				for (int i = 0; i < T_numTypes; ++i)
				{
					Data * const data = findAuction (s_dataVectors [i], id);
					if (data && !oldDataSet)
					{
						oldData = *data;
						oldData.header.highBidderId = playerId;
						oldData.header.timer = 0;
						oldData.header.flags &= ~(AUCTION_ACTIVE);
						oldDataSet = true;
						break;
					}
				}

				removeAuction (id);

				if (oldDataSet)
				{
					if (isVendor && playerOwns)
						addAuction (T_vendorStockroom, oldData);
					else
						addAuction (T_available, oldData);
				}
			}
		}
		else
		{
			CuiStringIdsAuction::bid_successful.localize (str);

			for (int i = 0; i < T_numTypes; ++i)
			{
				Data * const data = findAuction (s_dataVectors [i], id);
				if (data)
				{
					//@todo: set high bid value as well
					data->header.highBidderId = playerId;
					fireListChanged (i);
				}
			}
		}

		if (getAuctionToView () == id)
			setAuctionToView (NetworkId::cms_invalid);

		if (wasBuying)
			CuiAuctionManager::handleItemRetrieve (id);

		break;
	case ar_INVALID_AUCTIONER:
		if (wasBuying)
			CuiStringIdsAuction::err_buy_invalid_auctioner.localize (str);
		else
			CuiStringIdsAuction::err_bid_invalid_auctioner.localize (str);
		break;
	case ar_INVALID_ITEM_ID:
		if (wasBuying)
			CuiStringIdsAuction::err_buy_invalid_item.localize (str);
		else
			CuiStringIdsAuction::err_bid_invalid_item.localize (str);
		break;
	case ar_INVALID_CONTAINER_ID:
		if (wasBuying)
			CuiStringIdsAuction::err_buy_invalid_container.localize (str);
		else
			CuiStringIdsAuction::err_bid_invalid_container.localize (str);
		break;
	case ar_NOT_ENOUGH_MONEY:
		if (wasBuying)
			CuiStringIdsAuction::err_buy_not_enough_money.localize (str);
		else
			CuiStringIdsAuction::err_bid_not_enough_money.localize (str);
		break;
	case ar_INVALID_BID:
		if (wasBuying)
			CuiStringIdsAuction::err_buy_invalid.localize (str);
		else
			CuiStringIdsAuction::err_bid_invalid.localize (str);
		break;
	case ar_BID_OUTBID:
		if (wasBuying)
			CuiStringIdsAuction::err_buy_invalid.localize (str);
		else
			CuiStringIdsAuction::err_bid_outbid.localize (str);
		break;
	case ar_BID_REJECTED:
		if (wasBuying)
			CuiStringIdsAuction::err_buy_rejected.localize (str);
		else
			CuiStringIdsAuction::err_bid_rejected.localize (str);
		break;
	case ar_BID_TOO_HIGH:
		{
			CuiStringVariablesData data;
			data.digit_i = s_maxBid;
			CuiStringVariablesManager::process(CuiStringIdsAuction::err_bid_bid_too_high, data, str);
		}
		break;
	default:
		WARNING (true, ("AuctionManagerClient::receiveBidResponse unhandled result %d", resultCode));
		break;
	}

	Transceivers::bidResponse.emitMessage (Messages::BidResponse::Payload (id, Messages::ResultInfo (resultCode, str)));
}

//----------------------------------------------------------------------

void AuctionManagerClient::receiveRetrieveResponse             (const NetworkId & id, int resultCode)
{
	Unicode::String str;

	switch (resultCode)
	{
	case ar_OK:
		CuiStringIdsAuction::retrieve_successful.localize (str);
		removeAuction (id);
		s_details.erase (id);
		break;
	case ar_INVALID_AUCTIONER:
		CuiStringIdsAuction::err_retrieve_invalid_auctioner.localize (str);
		break;
	case ar_INVALID_ITEM_ID:
		CuiStringIdsAuction::err_retrieve_invalid_item.localize (str);
		break;
	case ar_INVALID_CONTAINER_ID:
		CuiStringIdsAuction::err_retrieve_invalid_container.localize (str);
		break;
//	case ar_NOT_ENOUGH_MONEY:
//		CuiStringIdsAuction::err_retrieve_not_enough_money.localize (str);
//		break;
	case ar_NOT_ITEM_OWNER:
		CuiStringIdsAuction::err_retrieve_not_owner.localize (str);
		break;
	case ar_INVENTORY_FULL:
		CuiStringIdsAuction::err_inventory_full.localize (str);
		break;
	case ar_ITEM_NOLONGER_EXISTS:
		CuiStringIdsAuction::err_retrieve_item_does_not_exist.localize(str);
		break;
	case ar_INVALID_ITEM_REIMBURSAL:
		CuiStringIdsAuction::err_retrieve_reimbursed.localize(str);
		break;
	default:
		WARNING (true, ("AuctionManagerClient::receiveRetrieveResponse unhandled result %d", resultCode));
		break;
	}

	Transceivers::retrieved.emitMessage (Messages::Retrieved::Payload (id, Messages::ResultInfo (resultCode, str)));
}

//----------------------------------------------------------------------

void AuctionManagerClient::receiveCancelLiveAuctionResponse    (const NetworkId & id, int resultCode)
{
	Unicode::String str;

	switch (resultCode)
	{
	case ar_OK:
		CuiStringIdsAuction::cancel_successful.localize (str);

		{
			bool playerOwns = false;
			const bool isVendor = isAtVendor (playerOwns);

			Data oldData;
			bool oldDataSet = false;

			for (int i = 0; i < T_numTypes; ++i)
			{
				Data * const data = findAuction (s_dataVectors [i], id);
				if (data && !oldDataSet)
				{
					oldData = *data;
					oldData.header.highBidderId = NetworkId::cms_invalid;
					oldData.header.timer = 0;
					oldData.header.flags &= ~(AUCTION_ACTIVE);
					oldDataSet = true;
					break;
				}
			}

			removeAuction (id);

			if (oldDataSet)
			{
				if (isVendor && playerOwns)
					addAuction (T_vendorStockroom, oldData);
				else
					addAuction (T_available, oldData);
			}
		}

		break;
	case ar_INVALID_AUCTIONER:
		CuiStringIdsAuction::err_cancel_invalid_auctioner.localize (str);
		break;
	case ar_INVALID_ITEM_ID:
		CuiStringIdsAuction::err_cancel_invalid_item.localize (str);
		break;
	case ar_NOT_ITEM_OWNER:
		CuiStringIdsAuction::err_cancel_not_owner.localize (str);
		break;
	case ar_AUCTION_ALREADY_COMPLETED:
		CuiStringIdsAuction::err_cancel_auction_already_completed.localize (str);
		break;
	default:
		WARNING (true, ("AuctionManagerClient::receiveCancelLiveAuctionResponse unhandled result %d", resultCode));
		break;
	}

	Transceivers::cancelLiveAuctionResponse.emitMessage (Messages::CancelLiveAuctionResponse::Payload (id, Messages::ResultInfo (resultCode, str)));
}

//----------------------------------------------------------------------

void AuctionManagerClient::receiveAcceptAuctionResponseMessage (const NetworkId & id, int resultCode)
{
	Unicode::String str;

	switch (resultCode)
	{
	case ar_OK:
		CuiStringIdsAuction::accept_bid_successful.localize (str);
		removeAuction (id);
		s_details.erase (id);
		break;
	case ar_INVALID_AUCTIONER:
		CuiStringIdsAuction::err_accept_bid_invalid_auctioner.localize (str);
		break;
	case ar_INVALID_ITEM_ID:
		CuiStringIdsAuction::err_accept_bid_invalid_item.localize (str);
		break;
	case ar_NOT_ITEM_OWNER:
		CuiStringIdsAuction::err_accept_bid_not_owner.localize (str);
		break;
	case ar_INVALID_BID:
		CuiStringIdsAuction::err_accept_bid_no_bids.localize (str);
		break;
	case ar_AUCTION_ALREADY_COMPLETED:
		CuiStringIdsAuction::err_accept_bid_auction_already_complete.localize (str);
		break;
	default:
		WARNING (true, ("AuctionManagerClient::receiveAcceptAuctionResponseMessage unhandled result %d", resultCode));
		break;
	}

	Transceivers::acceptBid.emitMessage (Messages::CancelLiveAuctionResponse::Payload (id, Messages::ResultInfo (resultCode, str)));
}

//----------------------------------------------------------------------

void AuctionManagerClient::receiveVendorOwnerResponseMessage   (int ownerResult, int resultCode, const NetworkId & containerId, const std::string & marketName)
{
	const Messages::VendorOwnerResponse::CodesName cn (Messages::VendorOwnerResponse::Codes (ownerResult, resultCode), marketName);
	Transceivers::vendorOwnerResponse.emitMessage (Messages::VendorOwnerResponse::Payload (containerId,  cn));
}

//----------------------------------------------------------------------

void AuctionManagerClient::receiveItemTypeList(const std::string & itemTypeListVersion, const ItemTypeListServer & itemTypeList)
{
	if (s_itemTypeListVersion != itemTypeListVersion)
	{
		s_itemTypeListVersion = itemTypeListVersion;
		s_itemTypeListServer = itemTypeList;

		s_itemTypeList.clear();

		for (ItemTypeListServer::const_iterator iter = itemTypeList.begin(); iter != itemTypeList.end(); ++iter)
		{
			ItemTypeNameList & itemTypeNameList = s_itemTypeList[iter->first];

			for (ItemTypeNameListServer::const_iterator iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2)
			{
				IGNORE_RETURN(itemTypeNameList.insert(std::make_pair(iter2->second.second.localize(), std::make_pair(iter2->first, iter2->second.second.getCanonicalRepresentation()))));
			}
		}

		Transceivers::itemTypeListChanged.emitMessage (s_itemTypeListVersion);
	}
}

//----------------------------------------------------------------------

void AuctionManagerClient::receiveResourceTypeList(const std::string & resourceTypeListVersion, const ResourceTypeListServer & resourceTypeList)
{
	if (s_resourceTypeListVersion != resourceTypeListVersion)
	{
		s_resourceTypeListVersion = resourceTypeListVersion;

		s_resourceTypeList.clear();

		for (ResourceTypeListServer::const_iterator iter = resourceTypeList.begin(); iter != resourceTypeList.end(); ++iter)
		{
			ResourceTypeNameList & resourceTypeNameList = s_resourceTypeList[iter->first];

			for (ResourceTypeNameListServer::const_iterator iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2)
			{
				resourceTypeNameList[*iter2] = static_cast<int>(Crc::calculate(iter2->c_str()));
			}
		}

		Transceivers::resourceTypeListChanged.emitMessage (s_resourceTypeListVersion);
	}
}

//----------------------------------------------------------------------

void AuctionManagerClient::getAuctions       (AuctionListType alt, DataVector & dv)
{
	int currentIndex = 0;
	bool hasMoreAuctions = false;
	int pageSize = 0;

	getAuctions (alt, dv, currentIndex, hasMoreAuctions, pageSize);
}

//----------------------------------------------------------------------

void AuctionManagerClient::getAuctions              (AuctionListType alt, DataVector & dv, int & currentIndex, bool & hasMoreAuctions, int & pageSize)
{
	if (alt >= T_all && alt < T_numTypes)
	{
		dv              = s_dataVectors     [alt];
		currentIndex    = s_currentIndices  [alt];
		hasMoreAuctions = s_hasMoreAuctions [alt];
		pageSize        = s_pageSizes       [alt];
	}
}

//----------------------------------------------------------------------

uint8 AuctionManagerClient::requestDetails       (const NetworkId & itemId)
{
	const GetAuctionDetails msg (itemId);
	GameNetwork::send (msg, true);

	if (Game::getSinglePlayer ())
	{
		Data * const data = findAuction (itemId);
		if (data)
		{
			data->simulateDetails (itemId);
			AuctionManagerClient::receiveDetails (data->details);
		}
	}

	return 0;
}

//----------------------------------------------------------------------

uint8 AuctionManagerClient::getMinTextFilterTokenLength()
{
	return s_minTextFilterTokenLength;
}

//----------------------------------------------------------------------

uint8 AuctionManagerClient::requestAuctions(int locationSearchType, int objectType, bool objectTypeExactMatch, int objectTemplateId, const Unicode::String & textFilterAll, const Unicode::String & textFilterAny, int priceFilterMin, int priceFilterMax, bool priceFilterIncludesFee, const std::list<AuctionQueryHeadersMessage::SearchCondition> & advancedSearch, AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny advancedSearchMatchAllAny, int startingIndex)
{
	if (startingIndex < 0)
		startingIndex = s_currentIndices [T_all];

	if (Game::getSinglePlayer ())
	{
		AuctionHeaderVector ahv;
		simulateHeaders (T_all, ahv, false, false, startingIndex);
		receiveHeaders (ahv, startingIndex, false);
		return 0;
	}

	static uint8 s_sequence = 0;

	++s_sequence;

	if (!s_sequence)
		++s_sequence;

	const AuctionQueryHeadersMessage msg (static_cast<AuctionQueryHeadersMessage::AuctionLocationSearch>(locationSearchType), ms_marketObjectId, s_sequence, AuctionQueryHeadersMessage::AST_ByAll, objectType, objectTypeExactMatch, objectTemplateId, textFilterAll, textFilterAny, priceFilterMin, priceFilterMax, priceFilterIncludesFee, advancedSearch, advancedSearchMatchAllAny, false, static_cast<uint16>(startingIndex));
	GameNetwork::send (msg, true);

	s_listRequestOutstanding [T_all] = true;

	return s_sequence;
}

//----------------------------------------------------------------------

uint8 AuctionManagerClient::requestSales (int startingIndex)
{
	if (startingIndex < 0)
		startingIndex = s_currentIndices [T_mySales];

	if (Game::getSinglePlayer ())
	{
		AuctionHeaderVector ahv;
		simulateHeaders     (T_mySales, ahv, true, false, startingIndex);
		receiveHeadersSales (ahv, startingIndex, false);
		return 0;
	}

	static uint8 s_sequence = 0;

	++s_sequence;

	if (!s_sequence)
		++s_sequence;

	const AuctionQueryHeadersMessage msg (AuctionQueryHeadersMessage::ALS_Galaxy, ms_marketObjectId, s_sequence, AuctionQueryHeadersMessage::AST_ByPlayerSales, 0, false, 0, Unicode::emptyString, Unicode::emptyString, 0, 0, false, std::list<AuctionQueryHeadersMessage::SearchCondition>(), AuctionQueryHeadersMessage::ASMAA_match_all, false, static_cast<uint16>(startingIndex));
	GameNetwork::send (msg, true);

	s_listRequestOutstanding [T_mySales] = true;

	return s_sequence;
}

//----------------------------------------------------------------------

uint8 AuctionManagerClient::requestBids (int startingIndex)
{
	if (startingIndex < 0)
		startingIndex = s_currentIndices [T_myBids];

	if (Game::getSinglePlayer ())
	{
		AuctionHeaderVector ahv;
		simulateHeaders     (T_myBids, ahv, false, false, startingIndex);
		receiveHeadersBids (ahv, startingIndex, false);
		return 0;
	}

	static uint8 s_sequence = 0;

	++s_sequence;

	if (!s_sequence)
		++s_sequence;

	const AuctionQueryHeadersMessage msg (AuctionQueryHeadersMessage::ALS_Galaxy, ms_marketObjectId, s_sequence, AuctionQueryHeadersMessage::AST_ByPlayerBids, 0, false, 0, Unicode::emptyString, Unicode::emptyString, 0, 0, false, std::list<AuctionQueryHeadersMessage::SearchCondition>(), AuctionQueryHeadersMessage::ASMAA_match_all, false, static_cast<uint16>(startingIndex));
	GameNetwork::send (msg, true);

	s_listRequestOutstanding [T_myBids] = true;

	return s_sequence;
}


//----------------------------------------------------------------------

uint8 AuctionManagerClient::requestAvailable (int startingIndex)
{
	if (startingIndex < 0)
		startingIndex = s_currentIndices [T_available];

	if (Game::getSinglePlayer ())
	{
		AuctionHeaderVector ahv;
		simulateHeaders         (T_available, ahv, true, true, startingIndex);
		receiveHeadersAvailable (ahv, startingIndex, false);
		return 0;
	}

	static uint8 s_sequence = 0;

	++s_sequence;

	if (!s_sequence)
		++s_sequence;

	const AuctionQueryHeadersMessage::AuctionLocationSearch loc = AuctionManagerClient::isAtCommodityMarket () ? AuctionQueryHeadersMessage::ALS_Galaxy : AuctionQueryHeadersMessage::ALS_Market;
	const AuctionQueryHeadersMessage msg (loc, ms_marketObjectId, s_sequence, AuctionQueryHeadersMessage::AST_ByPlayerStockroom, 0, false, 0, Unicode::emptyString, Unicode::emptyString, 0, 0, false, std::list<AuctionQueryHeadersMessage::SearchCondition>(), AuctionQueryHeadersMessage::ASMAA_match_all, ms_playerOwnsVendor, static_cast<uint16>(startingIndex));
	GameNetwork::send (msg, true);

	s_listRequestOutstanding [T_available] = true;

	return s_sequence;
}

//----------------------------------------------------------------------

uint8 AuctionManagerClient::requestVendorSelling     (bool sellerView, int locationSearchType, int objectType, bool objectTypeExactMatch, int objectTemplateId, const Unicode::String & textFilterAll, const Unicode::String & textFilterAny, int priceFilterMin, int priceFilterMax, bool priceFilterIncludesFee, const std::list<AuctionQueryHeadersMessage::SearchCondition> & advancedSearch, AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny advancedSearchMatchAllAny, int startingIndex)
{
	if (startingIndex < 0)
		startingIndex = s_currentIndices [T_vendorSelling];

	if (Game::getSinglePlayer ())
	{
		AuctionHeaderVector ahv;
		simulateHeaders         (T_vendorSelling, ahv, sellerView, true, startingIndex);
		receiveHeadersVendorSelling (ahv, startingIndex, false);
		return 0;
	}

	static uint8 s_sequence = 0;

	++s_sequence;

	if (!s_sequence)
		++s_sequence;

	const AuctionQueryHeadersMessage msg (static_cast<AuctionQueryHeadersMessage::AuctionLocationSearch>(locationSearchType), ms_marketObjectId, s_sequence, AuctionQueryHeadersMessage::AST_ByVendorSelling, objectType, objectTypeExactMatch, objectTemplateId, textFilterAll, textFilterAny, priceFilterMin, priceFilterMax, priceFilterIncludesFee, advancedSearch, advancedSearchMatchAllAny, ms_playerOwnsVendor, static_cast<uint16>(startingIndex));
	GameNetwork::send (msg, true);

	s_listRequestOutstanding [T_vendorSelling] = true;

	return s_sequence;
}

//----------------------------------------------------------------------

uint8 AuctionManagerClient::requestVendorOffers      (bool playerOffers, int locationSearchType, int objectType, bool objectTypeExactMatch, int objectTemplateId, const Unicode::String & textFilterAll, const Unicode::String & textFilterAny, int priceFilterMin, int priceFilterMax, bool priceFilterIncludesFee, const std::list<AuctionQueryHeadersMessage::SearchCondition> & advancedSearch, AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny advancedSearchMatchAllAny, int startingIndex)
{
	if (startingIndex < 0)
		startingIndex = s_currentIndices [T_vendorOffers];

	if (Game::getSinglePlayer ())
	{
		AuctionHeaderVector ahv;
		simulateHeaders         (T_vendorOffers, ahv, playerOffers, true, startingIndex);
		receiveHeadersVendorOffers (ahv, startingIndex, false);
		return 0;
	}

	static uint8 s_sequence = 0;

	++s_sequence;

	if (!s_sequence)
		++s_sequence;

	if (playerOffers)
	{
		const AuctionQueryHeadersMessage msg (static_cast<AuctionQueryHeadersMessage::AuctionLocationSearch>(locationSearchType), ms_marketObjectId, s_sequence, AuctionQueryHeadersMessage::AST_ByPlayerOffersToVendor, objectType, objectTypeExactMatch, objectTemplateId, textFilterAll, textFilterAny, priceFilterMin, priceFilterMax, priceFilterIncludesFee, advancedSearch, advancedSearchMatchAllAny, ms_playerOwnsVendor, static_cast<uint16>(startingIndex));
		GameNetwork::send (msg, true);
	}
	else
	{
		const AuctionQueryHeadersMessage msg (static_cast<AuctionQueryHeadersMessage::AuctionLocationSearch>(locationSearchType), ms_marketObjectId, s_sequence, AuctionQueryHeadersMessage::AST_ByVendorOffers, objectType, objectTypeExactMatch, objectTemplateId, textFilterAll, textFilterAny, priceFilterMin, priceFilterMax, priceFilterIncludesFee, advancedSearch, advancedSearchMatchAllAny, ms_playerOwnsVendor, static_cast<uint16>(startingIndex));
		GameNetwork::send (msg, true);
	}

	s_listRequestOutstanding [T_vendorOffers] = true;

	return s_sequence;
}

//----------------------------------------------------------------------

uint8 AuctionManagerClient::requestVendorStockroom   (int locationSearchType, int objectType, bool objectTypeExactMatch, int objectTemplateId, const Unicode::String & textFilterAll, const Unicode::String & textFilterAny, int priceFilterMin, int priceFilterMax, bool priceFilterIncludesFee, const std::list<AuctionQueryHeadersMessage::SearchCondition> & advancedSearch, AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny advancedSearchMatchAllAny, int startingIndex)
{
	if (startingIndex < 0)
		startingIndex = s_currentIndices [T_vendorStockroom];

	if (Game::getSinglePlayer ())
	{
		AuctionHeaderVector ahv;
		simulateHeaders         (T_vendorStockroom, ahv, true, true, startingIndex);
		receiveHeadersVendorStockroom (ahv, startingIndex, false);
		return 0;
	}

	static uint8 s_sequence = 0;

	++s_sequence;

	if (!s_sequence)
		++s_sequence;

	const AuctionQueryHeadersMessage msg (static_cast<AuctionQueryHeadersMessage::AuctionLocationSearch>(locationSearchType), ms_marketObjectId, s_sequence, AuctionQueryHeadersMessage::AST_ByVendorStockroom, objectType, objectTypeExactMatch, objectTemplateId, textFilterAll, textFilterAny, priceFilterMin, priceFilterMax, priceFilterIncludesFee, advancedSearch, advancedSearchMatchAllAny, ms_playerOwnsVendor, static_cast<uint16>(startingIndex));
	GameNetwork::send (msg, true);

	s_listRequestOutstanding [T_vendorStockroom] = true;

	return s_sequence;
}

//----------------------------------------------------------------------

uint8 AuctionManagerClient::requestLocations(int locationSearchType, int objectType, bool objectTypeExactMatch, int objectTemplateId, const Unicode::String & textFilterAll, const Unicode::String & textFilterAny, int priceFilterMin, int priceFilterMax, bool priceFilterIncludesFee, const std::list<AuctionQueryHeadersMessage::SearchCondition> & advancedSearch, AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny advancedSearchMatchAllAny, int startingIndex)
{
	if (startingIndex < 0)
		startingIndex = s_currentIndices [T_location];

	if (Game::getSinglePlayer ())
	{
		AuctionHeaderVector ahv;
		simulateHeaders (T_all, ahv, false, false, startingIndex);
		receiveHeaders (ahv, startingIndex, false);
		return 0;
	}

	static uint8 s_sequence = 0;

	++s_sequence;

	if (!s_sequence)
		++s_sequence;

	AuctionQueryHeadersMessage const msg(static_cast<AuctionQueryHeadersMessage::AuctionLocationSearch>(locationSearchType), ms_marketObjectId, s_sequence, AuctionQueryHeadersMessage::AST_ByVendorSelling, objectType, objectTypeExactMatch, objectTemplateId, textFilterAll, textFilterAny, priceFilterMin, priceFilterMax, priceFilterIncludesFee, advancedSearch, advancedSearchMatchAllAny, false, static_cast<uint16>(startingIndex));
	GameNetwork::send (msg, true);

	s_listRequestOutstanding [T_location] = true;

	return s_sequence;
}

//----------------------------------------------------------------------

void AuctionManagerClient::requestItemTypeList()
{
	GenericValueTypeMessage<std::string> const msg("CommoditiesItemTypeListRequest", s_itemTypeListVersion);
	GameNetwork::send (msg, true);
}

//----------------------------------------------------------------------

const AuctionManagerClient::ItemTypeNameList & AuctionManagerClient::getItemTypeNameList(int got)
{
	static ItemTypeNameList emptyList;

	ItemTypeList::iterator iter = s_itemTypeList.find(got);
	if (iter != s_itemTypeList.end())
		return iter->second;

	return emptyList;
}

//----------------------------------------------------------------------

const AuctionManagerClient::ItemTypeNameListServer & AuctionManagerClient::getItemTypeNameListServer(int got)
{
	static ItemTypeNameListServer emptyList;

	ItemTypeListServer::iterator iter = s_itemTypeListServer.find(got);
	if (iter != s_itemTypeListServer.end())
		return iter->second;

	return emptyList;
}

//----------------------------------------------------------------------

const std::string & AuctionManagerClient::getItemTypeListVersion()
{
	return s_itemTypeListVersion;
}

//----------------------------------------------------------------------

void AuctionManagerClient::requestResourceTypeList()
{
	GenericValueTypeMessage<std::string> const msg("CommoditiesResourceTypeListRequest", s_resourceTypeListVersion);
	GameNetwork::send (msg, true);
}

//----------------------------------------------------------------------

const AuctionManagerClient::ResourceTypeNameList & AuctionManagerClient::getResourceTypeNameList(int resourceClassCrc)
{
	static ResourceTypeNameList emptyList;

	ResourceTypeList::iterator iter = s_resourceTypeList.find(resourceClassCrc);
	if (iter != s_resourceTypeList.end())
		return iter->second;

	return emptyList;
}

//----------------------------------------------------------------------

const std::string & AuctionManagerClient::getResourceTypeListVersion()
{
	return s_resourceTypeListVersion;
}

//----------------------------------------------------------------------

bool AuctionManagerClient::getAuctionData       (const NetworkId & itemId, Data & _data)
{
	const Data * const data = findAuction (itemId);

	if (data)
	{
		_data = *data;
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

void AuctionManagerClient::makeBid              (const NetworkId & itemId, int bid, int proxyBid, bool isBuying)
{
	AuctionManagerClientHistory::bidAdd (itemId, isBuying);

	const BidAuctionMessage msg (itemId, bid, proxyBid);
	GameNetwork::send (msg, true);
}

//----------------------------------------------------------------------

void AuctionManagerClient::sellItem (const NetworkId & container, const NetworkId & item, const Unicode::String & itemLocalizedName, int bid, bool instantSale, int timeLimitSecs, const Unicode::String & desc, bool isPremium, bool isVendorStockroomTransfer)
{
	AuctionManagerClientHistory::saleAdd (item, !instantSale);

	if (instantSale)
	{
		const CreateImmediateAuctionMessage	msg (item, itemLocalizedName, container, bid, timeLimitSecs, desc, isPremium, isVendorStockroomTransfer);
		GameNetwork::send (msg, true);
	}
	else
	{
		const CreateAuctionMessage msg (item, itemLocalizedName, container, bid, timeLimitSecs, desc, isPremium);
		GameNetwork::send (msg, true);
	}
}

//----------------------------------------------------------------------

void AuctionManagerClient::relistItemsFromStock(const std::vector<NetworkId> & itemIds, int timeLimitSecs)
{
	if (itemIds.empty())
		return;

	// items bought from a vendor offer cannot be relisted because
	// no sell price has been entered for the items; the price on
	// the items is the offer price, which is probably not the price
	// the merchant wants to sell at; these items must be relisted
	// using the "sell" action to manually enter an initial sell price
	std::vector<NetworkId> finalItemIds;

	// if relisting the item from stock room, see if it's an older
	// item that is stored in the commodities system with a string
	// id as the item name, and if yes, replace the item name with
	// the localized name
	std::vector<Unicode::String> itemLocalizedNames;
	Unicode::String itemLocalizedName;
	for (std::vector<NetworkId>::const_iterator itemIdsIter = itemIds.begin(); itemIdsIter != itemIds.end(); ++itemIdsIter)
	{
		AuctionManagerClientData const * const data = AuctionManagerClient::findAuction(*itemIdsIter);
		if (data)
		{
			// skip items bought from a vendor offer
			if (data->header.flags & AUCTION_OFFERED_ITEM)
				continue;

			// split the auction item name string into its localized components
			std::vector<Unicode::String> undecodedComponents;
			std::vector<Unicode::String> decodedComponents;

			// if any of the component got localized, then reconstruct
			// the auction item name using the localized name and send
			// it back to the commodities server so the item will get
			// relisted with its localized name
			if (0 < data->decodeAuctionItemNameString(undecodedComponents, decodedComponents))
			{
				itemLocalizedName.clear();
				
				for (std::vector<Unicode::String>::const_iterator iter = decodedComponents.begin(); iter != decodedComponents.end(); ++iter)
				{
					if (itemLocalizedName.size() > 0)
						itemLocalizedName += uint16(0);

					itemLocalizedName += *iter;
				}

				itemLocalizedNames.push_back(itemLocalizedName);
			}
			else
			{
				itemLocalizedNames.push_back(Unicode::String());
			}
		}
		else
		{
			itemLocalizedNames.push_back(Unicode::String());
		}

		finalItemIds.push_back(*itemIdsIter);
	}

	if (!finalItemIds.empty())
	{
		GenericValueTypeMessage<std::pair<std::pair<std::vector<NetworkId>, std::vector<Unicode::String> >, int> > const msg("RelistItemsFromStockMessage", std::make_pair(std::make_pair(finalItemIds, itemLocalizedNames), timeLimitSecs));
		GameNetwork::send(msg, true);
	}

	if (finalItemIds.size() != itemIds.size())
		CuiMessageBox::createInfoBox(CuiStringIdsAuction::err_cannot_relist_offered_items.localize());
}

//----------------------------------------------------------------------

void AuctionManagerClient::withdrawItem (const NetworkId & itemId)
{
	const CancelLiveAuctionMessage msg (itemId);
	GameNetwork::send (msg, true);

	Transceivers::withdrawRequested.emitMessage (itemId);
}

//----------------------------------------------------------------------

void AuctionManagerClient::acceptBid (const NetworkId & /*itemId*/)
{
#if 0
	const AcceptAuctionMessage msg (itemId);
	GameNetwork::send (msg, true);
#endif
}

//----------------------------------------------------------------------

void AuctionManagerClient::retrieve (const NetworkId & itemId, const NetworkId & marketId)
{
	const RetrieveAuctionItemMessage msg (itemId, marketId);
	GameNetwork::send (msg, true);
	Transceivers::retrieveRequested.emitMessage (itemId);
}

//----------------------------------------------------------------------

void AuctionManagerClient::setMarketObjectId        (const NetworkId & id)
{
	ms_marketObjectId = id;
}

//----------------------------------------------------------------------

void AuctionManagerClient::setMarketRegionName (const std::string & regionName)
{
	ms_marketRegionName = regionName;
}

//----------------------------------------------------------------------

bool AuctionManagerClient::getMarketLocationStrings (std::string & planet, std::string & city, std::string & market)
{
	if (Game::getSinglePlayer ())
	{
		planet = "planetMercury";
		city   = "atlantis";
		market = "Jack's Big-Ass Market";
		return true;
	}

	const ClientObject * const obj = dynamic_cast<ClientObject *>(ms_marketObjectId.getObject ());

	if (obj)
	{
		planet = Game::getSceneId ();
		city   = ms_marketRegionName;
		market = Unicode::wideToNarrow (obj->getLocalizedName ());
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

void AuctionManagerClient::setAuctionToBidOn        (const NetworkId & auctionToBidOn)
{
	ms_auctionToBidOn = auctionToBidOn;
	Transceivers::auctionToBidOnChanged.emitMessage (auctionToBidOn);
}

//----------------------------------------------------------------------

void AuctionManagerClient::setAuctionToSetPriceOn   (const NetworkId & auctionToSetPriceOn)
{
	ms_auctionToSetPriceOn = auctionToSetPriceOn;
	Transceivers::auctionToSetPriceOnChanged.emitMessage (auctionToSetPriceOn);
}

//----------------------------------------------------------------------

void AuctionManagerClient::setAuctionToView (const NetworkId & auctionToView)
{
	ms_auctionToView = auctionToView;
	Transceivers::auctionToViewChanged.emitMessage (auctionToView);
}

//----------------------------------------------------------------------

void AuctionManagerClient::traverseAuctionToView     (bool forward)
{
	Transceivers::traverseAuctionToView.emitMessage (forward);
}

//----------------------------------------------------------------------

bool AuctionManagerClient::isAtCommodityMarket       ()
{
	return ms_atCommodityMarket;
}

//----------------------------------------------------------------------

bool AuctionManagerClient::isAtVendor                (bool & playerOwns)
{
	playerOwns = ms_playerOwnsVendor;
	return ms_atVendor;
}

//----------------------------------------------------------------------

void AuctionManagerClient::setIsAtCommodityMarket       (bool b)
{
	ms_atCommodityMarket = b;
	if (ms_atCommodityMarket)
		setIsAtVendor (false, false);

}
//----------------------------------------------------------------------

void AuctionManagerClient::setIsAtVendor                (bool b, bool playerOwns)
{
	ms_playerOwnsVendor = playerOwns;
	ms_atVendor = b;
	if (ms_atVendor)
		setIsAtCommodityMarket (false);
}

//----------------------------------------------------------------------

void AuctionManagerClient::setItemToSellFromStock    (const NetworkId & itemId)
{
	ms_itemsToSellFromStock.clear();
	ms_itemsToSellFromStock.push_back(itemId);
}

//----------------------------------------------------------------------

void AuctionManagerClient::setItemsToSellFromStock    (const std::vector<NetworkId> & itemIds)
{
	ms_itemsToSellFromStock = itemIds;
}

//----------------------------------------------------------------------

namespace AuctionManagerClientUtils
{
	std::string getLocationString(const std::string &str)
	{
		size_t poundPos = str.rfind('#');
		if (poundPos != str.npos)
		{
			return str.substr(0, poundPos);
		}
		return str;
	}

	Vector getCoordinatesVector(const std::string &str)
	{
		char tmp[512];
		strcpy(tmp, str.c_str());
		char *poundLoc = strstr(tmp, "#");
		if (poundLoc != NULL)
		{
			char *commaLoc = strstr(poundLoc, ",");
			if (commaLoc != NULL)
			{
				int x = atoi(poundLoc + 1);
				int z = atoi(commaLoc + 1);
				return Vector((float)x, 0.0f, (float)z);
			}
		}
		Vector emptyVector;
		return emptyVector;
	}
}

//----------------------------------------------------------------------

bool AuctionManagerClient::localizeLocation          (const std::string & location, Unicode::String & result, bool compactFormat, bool showObjectName)
{
	Unicode::String planet;
	Unicode::String region;
	Unicode::String objectName;

	if (!localizeLocation (location, planet, region, objectName))
	{
		result = Unicode::narrowToWide (location);
		return false;
	}

	if (compactFormat)
	{
		if (!planet.empty ())
			result += planet;

		if (!region.empty ())
		{
			if (!result.empty ())
			{
				result.push_back (',');
				result.push_back (' ');
			}

			result += region;
		}

		if (showObjectName)
		{
			if (!objectName.empty ())
			{
				if (!result.empty ())
				{
					result.push_back (',');
					result.push_back (' ');
				}
				result += objectName;
			}
		}
	}
	else
	{
		static const Unicode::String contrast = Unicode::narrowToWide ("\\#pcontrast1 ");
		static const Unicode::String indent   = Unicode::narrowToWide (": \\>128");
		static const Unicode::String endl     = Unicode::narrowToWide ("\\>000\\#.\n");
		result += CuiStringIdsAuction::loc_planet.localize ()      + indent + contrast + planet     + endl;
		result += CuiStringIdsAuction::loc_region.localize ()      + indent + contrast + region     + endl;

		if (showObjectName)
			result += CuiStringIdsAuction::loc_market_name.localize () + indent + contrast + objectName + endl;
	}

	return true;
}

//----------------------------------------------------------------------

bool AuctionManagerClient::separateLocation(const std::string & location, std::string & planet, std::string & region, std::string & name, NetworkId & id)
{
	return AuctionManager::separateLocation(location, planet, region, name, id);
}

//----------------------------------------------------------------------

bool AuctionManagerClient::localizeLocation(const std::string & location, Unicode::String & planet, Unicode::String & region, Unicode::String & objectName)
{
	std::string  narrow_planet;
	std::string  narrow_region;
	std::string  narrow_name;
	NetworkId id;

	AuctionManagerClient::separateLocation (location, narrow_planet, narrow_region, narrow_name, id);

	if (!narrow_planet.empty ())
		planet = StringId ("planet_n", narrow_planet).localize ();

	if (!narrow_region.empty ())
		region = StringId::decodeString (Unicode::narrowToWide (narrow_region));

	if (!narrow_name.empty ())
		objectName = StringId::decodeString (Unicode::narrowToWide (narrow_name));

	return true;
}

//----------------------------------------------------------------------

void AuctionManagerClient::update                    (float elapsedTime)
{
	s_timeSinceDecrementAuctionTimes += elapsedTime;

	if (s_timeSinceDecrementAuctionTimes >= 1.0f)
	{
		const int decrement = static_cast<int>(s_timeSinceDecrementAuctionTimes);

		decrementTimeouts (s_dataVectors [T_myBids],  decrement);
		decrementTimeouts (s_dataVectors [T_mySales], decrement);
		decrementTimeouts (s_dataVectors [T_all],     decrement);

		s_timeSinceDecrementAuctionTimes -= static_cast<float>(decrement);
	}
}

//----------------------------------------------------------------------

bool AuctionManagerClient::hasDetails(NetworkId const & itemId)
{
	return s_details.find(itemId) != s_details.end();
}

//----------------------------------------------------------------------

void AuctionManagerClient::getTooltipsFromDetails(NetworkId const & itemId, Unicode::String & tooltip)
{
	DetailsMap::const_iterator itDetails = s_details.find(itemId);
	
	if (itDetails != s_details.end()) 
	{
		Details const & details = itDetails->second;

		ObjectAttributeManager::formatAttributes(details.propertyList, tooltip, NULL, NULL, false, true);
		
		if (!details.userDescription.empty()) 
		{
			tooltip.append(1, '\n');
			tooltip.append(details.userDescription);
		}

		if (tooltip.empty()) 
		{
			ObjectAttributeManager::formatAttributes(details.propertyList, tooltip, NULL, NULL, false, false);
		}
	}
}


//======================================================================
