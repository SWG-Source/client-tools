//======================================================================
//
// SwgCuiAuctionListPane.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgclientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAuctionListPane.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIPopupMenu.h"
#include "UITable.h"
#include "UIText.h"
#include "UIUtils.h"
#include "UnicodeUtils.h"
#include "clientGame/AuctionManagerClient.h"
#include "clientGame/AuctionManagerClientData.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiAuctionManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIdsAuction.h"
#include "clientUserInterface/CuiSettings.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedGame/AuctionManager.h"
#include "sharedGame/Waypoint.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/CachedNetworkId.h"
#include "swgClientUserInterface/SwgCuiAuctionDetails.h"
#include "swgClientUserInterface/SwgCuiAuctionFilter.h"
#include "swgClientUserInterface/SwgCuiAuctionList.h"
#include "swgClientUserInterface/SwgCuiAuctionListView.h"
#include "swgClientUserInterface/SwgCuiAuctionPaneTypes.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

//----------------------------------------------------------------------

namespace 
{
	//-- setup the required buttons for each type of list

	enum ObjectFlags
	{
		OF_filter          = 0x0001,
		OF_bid             = 0x0002,
		OF_withdraw        = 0x0004,
		OF_accept          = 0x0008,
		OF_retrieve        = 0x0010,
		OF_sellFromStock   = 0x0020,
		OF_sell            = 0x0040,
		OF_changePrice     = 0x0080,
		OF_reject          = 0x0100,
		OF_offer           = 0x0200,
		OF_createWaypoint  = 0x0400,
		OF_relistFromStock = 0x0800
	};

	const int s_objectFlags [SwgCuiAuctionPaneTypes::T_numTypes] =
	{
		OF_filter   | OF_bid,                                // SwgCuiAuctionPaneTypes::T_all
		OF_withdraw | OF_accept        | OF_sell,            // SwgCuiAuctionPaneTypes::T_mySales
		OF_bid,                                              // SwgCuiAuctionPaneTypes::T_myBids
		OF_retrieve | OF_createWaypoint,                     // SwgCuiAuctionPaneTypes::T_available
		OF_filter   | OF_withdraw      | OF_sell,            // SwgCuiAuctionPaneTypes::T_vendorSellerSelling
		OF_filter   | OF_bid           | OF_reject,          // SwgCuiAuctionPaneTypes::T_vendorSellerOffers
		OF_filter   | OF_sellFromStock | OF_retrieve | OF_relistFromStock, // SwgCuiAuctionPaneTypes::T_vendorSellerStockroom
		OF_bid      | OF_filter,                             // SwgCuiAuctionPaneTypes::T_vendorBuyerSelling
		OF_offer    | OF_retrieve,                           // SwgCuiAuctionPaneTypes::T_vendorBuyerOffers
		OF_filter   | OF_createWaypoint                      // SwgCuiAuctionPaneTypes::T_location
	};
	
	namespace PopupData
	{
		const std::string details         = "details";
		const std::string buy             = "buy";
		const std::string place_bid       = "place_bid";
		const std::string sell_new        = "sell_new";
		const std::string sell_from_stock = "sell_from_stock";
		const std::string relist_from_stock = "relist_from_stock";
		const std::string offer           = "offer";
		const std::string reject          = "reject";
		const std::string withdraw        = "withdraw";
		const std::string accept_bid      = "accept_bid";
		const std::string retrieve        = "retrieve";
		const std::string createWaypoint  = "createWaypoint";
	}

	namespace Properties
	{
		const UILowerString AuctionId = UILowerString ("AuctionId");
	}

	namespace Settings
	{
		const std::string filterType = "filterType";
	}

	typedef SwgCuiAuctionListPane::NetworkIdVector NetworkIdVector;

	NetworkIdVector getIdsFromPopup (UIPopupMenu & pop)
	{
		Unicode::String prop;
		pop.GetProperty (Properties::AuctionId, prop);

		Unicode::UnicodeStringVector usv;
		Unicode::tokenize (prop, usv);

		NetworkIdVector niv;
		niv.reserve (usv.size ());

		for (Unicode::UnicodeStringVector::const_iterator it = usv.begin (); it != usv.end (); ++it)
		{
			niv.push_back (NetworkId (Unicode::wideToNarrow (*it)));
		}

		return niv;
	}

	void setIdsForPopup (UIPopupMenu & pop, const NetworkIdVector & niv)
	{
		std::string str;

		for (NetworkIdVector::const_iterator it = niv.begin (); it != niv.end (); ++it)
		{
			str += (*it).getValueString ();
			str.push_back (' ');
		}

		pop.SetPropertyNarrow (Properties::AuctionId, str);
	}	

}

//----------------------------------------------------------------------

SwgCuiAuctionListPane::SwgCuiAuctionListPane (const char * const debugName, UIPage & page, int type, SwgCuiAuctionListBase & listRoot, bool const disableBuyInDetails) :
CuiMediator           (debugName, page),
m_listRoot            (&listRoot),
m_callback            (new MessageDispatch::Callback),
m_filter              (0),
m_type                (type),
m_auctionListType     (SwgCuiAuctionPaneTypes::getAuctionListType (type)),
m_view                (0),
m_buttonBid           (0),
m_buttonDetails       (0),
m_buttonWithdraw      (0),
m_buttonAccept        (0),
m_buttonRetrieve      (0),
m_buttonSell          (0),
m_buttonSellFromStock (0),
m_buttonRelistFromStock(0),
m_buttonOffer         (0),
m_buttonChangePrice   (0),
m_buttonReject        (0),
m_buttonCreateWaypoint(0),
m_textListStatus      (0),
m_textPaging          (0),
m_buttonPagePrev      (0),
m_buttonPageNext      (0),
m_disableBuyInDetails(disableBuyInDetails)
{
	const int objectFlags = s_objectFlags [m_type];

	if ((objectFlags & OF_filter) != 0)
	{
		UIPage * pageFilter = 0;
		getCodeDataObject (TUIPage,       pageFilter,          "pageFilter");

		if (pageFilter)
		{
			const std::string & filterDebugName = getMediatorDebugName () + "_SwgCuiAuctionFilter";
			m_filter = new SwgCuiAuctionFilter (*pageFilter, filterDebugName.c_str ());
			m_filter->fetch ();
		}
	}	
	
	{
		UITable * table = 0;
		getCodeDataObject (TUITable,          table,          "table");		
		m_view =  new View (*this, *table,           m_filter,       type);
	}

	//-- optional widget
	getCodeDataObject (TUIText,               m_textListStatus,      "textListStatus", true);

	if (m_textListStatus)
	{
		m_textListStatus->SetPreLocalized (true);
		m_textListStatus->SetVisible (false);
	}

	if ((objectFlags & OF_bid)      != 0)
		getCodeDataObject (TUIButton,         m_buttonBid,           "buttonBid");

	if ((objectFlags & OF_withdraw) != 0)
		getCodeDataObject (TUIButton,         m_buttonWithdraw,      "buttonWithdraw");

	if ((objectFlags & OF_accept)   != 0)
		getCodeDataObject (TUIButton,         m_buttonAccept,        "buttonAccept");

	if ((objectFlags & OF_retrieve) != 0)
		getCodeDataObject (TUIButton,         m_buttonRetrieve,      "buttonRetrieve");

	if ((objectFlags & OF_sell)     != 0)
		getCodeDataObject (TUIButton,         m_buttonSell,          "buttonSell");

	if ((objectFlags & OF_sellFromStock)     != 0)
		getCodeDataObject (TUIButton,         m_buttonSellFromStock, "buttonSellFromStock");

	if ((objectFlags & OF_relistFromStock)   != 0)
		getCodeDataObject (TUIButton,         m_buttonRelistFromStock, "buttonRelistFromStock");

	if ((objectFlags & OF_offer)     != 0)
		getCodeDataObject (TUIButton,         m_buttonOffer,         "buttonOffer");

	if ((objectFlags & OF_changePrice)     != 0)
		getCodeDataObject (TUIButton,         m_buttonChangePrice,   "buttonChangePrice");

	if ((objectFlags & OF_reject)     != 0)
		getCodeDataObject (TUIButton,         m_buttonReject,        "buttonReject");

	if ((objectFlags & OF_createWaypoint) != 0)
		getCodeDataObject (TUIButton,         m_buttonCreateWaypoint,"buttonCreateWaypoint");

	getCodeDataObject (TUIText,           m_textPaging,        "textPaging",     true);
	getCodeDataObject (TUIButton,         m_buttonPagePrev,    "buttonPagePrev", true);
	getCodeDataObject (TUIButton,         m_buttonPageNext,    "buttonPageNext", true);

	if (m_buttonBid)
		registerMediatorObject (*m_buttonBid,       true);
	if (m_buttonWithdraw)
		registerMediatorObject (*m_buttonWithdraw,  true);
	if (m_buttonAccept)
		registerMediatorObject (*m_buttonAccept,    true);
	if (m_buttonRetrieve)
		registerMediatorObject (*m_buttonRetrieve,  true);
	if (m_buttonSell)
		registerMediatorObject (*m_buttonSell,      true);
	if (m_buttonSellFromStock)
		registerMediatorObject (*m_buttonSellFromStock,    true);
	if (m_buttonRelistFromStock)
		registerMediatorObject (*m_buttonRelistFromStock,  true);
	if (m_buttonOffer)
		registerMediatorObject (*m_buttonOffer,            true);
	if (m_buttonChangePrice)
		registerMediatorObject (*m_buttonChangePrice,      true);
	if (m_buttonReject)
		registerMediatorObject (*m_buttonReject,           true);
	if (m_buttonCreateWaypoint)
		registerMediatorObject (*m_buttonCreateWaypoint,   true);
	if (m_buttonPagePrev)
		registerMediatorObject (*m_buttonPagePrev,   true);
	if (m_buttonPageNext)
		registerMediatorObject (*m_buttonPageNext,   true);
	if (m_textPaging)
		m_textPaging->SetPreLocalized (true);
}

//----------------------------------------------------------------------

SwgCuiAuctionListPane::~SwgCuiAuctionListPane ()
{
	delete m_callback;
	m_callback = 0;

	if (m_filter)
	{
		m_filter->release ();
		m_filter = 0;
	}

	delete m_view;
	m_view = 0;

	m_buttonBid = 0;
}

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::performActivate ()
{
	if (m_filter)
		m_filter->activate ();

	getPage ().SetEnabled (true);

	m_view->setActive (true);

	requestUpdate (true, 0);

	m_callback->connect (*this, &SwgCuiAuctionListPane::onTraverseAuctionToView, static_cast<AuctionManagerClient::Messages::TraverseAuctionToView*> (0));

	m_callback->connect (m_view->getTransceiverDoubleClick      (), *this, &SwgCuiAuctionListPane::onViewDoubleClick);
	m_callback->connect (m_view->getTransceiverSelectionChanged (), *this, &SwgCuiAuctionListPane::onViewSelectionChanged);
	m_callback->connect (m_view->getTransceiverContextRequest   (), *this, &SwgCuiAuctionListPane::onViewContextRequest);

	setIsUpdating (true);

	updateButtonStates ();
}

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::performDeactivate ()
{
	UIManager::gUIManager ().PopContextWidgets (0);

	if (m_filter)
		m_filter->deactivate ();

	m_view->setActive (false);
	m_callback->disconnect (*this, &SwgCuiAuctionListPane::onTraverseAuctionToView, static_cast<AuctionManagerClient::Messages::TraverseAuctionToView*> (0));	
	m_callback->disconnect (m_view->getTransceiverSelectionChanged (), *this, &SwgCuiAuctionListPane::onViewSelectionChanged);
	m_callback->disconnect (m_view->getTransceiverDoubleClick (), *this, &SwgCuiAuctionListPane::onViewDoubleClick);
	m_callback->disconnect (m_view->getTransceiverContextRequest   (), *this, &SwgCuiAuctionListPane::onViewContextRequest);

	setIsUpdating (false);
}

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::requestUpdate (bool optional, int incrementIndex)
{
	int currentIndex = 0;
	bool hasMoreAuctions = false;
	int pageSize = 0;
	static AuctionManagerClient::DataVector dv;
	AuctionManagerClient::getAuctions  (static_cast<AuctionManagerClient::AuctionListType>(m_auctionListType), dv, currentIndex, hasMoreAuctions, pageSize);
	
	bool shouldUpdate = !optional || 
		(m_auctionListType != AuctionManagerClient::T_all && 
		m_auctionListType != AuctionManagerClient::T_vendorSelling);
	
	if (!shouldUpdate)
	{
		//-- no list yet received, don't request one
		if (!AuctionManagerClient::isListValid (static_cast<AuctionManagerClient::AuctionListType>(m_auctionListType)))
		{
			if (!m_textListStatus || !m_filter)
				shouldUpdate = true;
		}
	}
	
	if (shouldUpdate)
	{
		// if any of the text/price filter value has changed, start searching at the beginning
		if (((currentIndex + incrementIndex) > 0) && m_filter && m_filter->hasTextPriceFilterValuesChangedSinceCheckpoint ())
			m_view->requestUpdate (0);
		else
			m_view->requestUpdate (currentIndex + incrementIndex);
	}

	updateTextListStatus ();
}

//----------------------------------------------------------------------

const NetworkId & SwgCuiAuctionListPane::findSelectedAuction (bool warn) const
{
	NOT_NULL (m_view);
	return m_view->findSelectedAuction (warn);
}

//----------------------------------------------------------------------

const SwgCuiAuctionListPane::NetworkIdVector & SwgCuiAuctionListPane::findSelectedAuctions      (bool warn) const
{
	NOT_NULL (m_view);
	return m_view->findSelectedAuctions (warn);
}

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::showDetails (const NetworkId & auctionId) const
{
	if (!auctionId.isValid ())
		return;

	AuctionManagerClient::setAuctionToView (auctionId);
	CuiMediator * const mediator = CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_AuctionDetails);
	SwgCuiAuctionDetails * const auctionDetails = static_cast<SwgCuiAuctionDetails *>(mediator);
	if (auctionDetails != 0)
	{
		auctionDetails->setBuyButtonEnabled(!m_disableBuyInDetails);
	}
}


//----------------------------------------------------------------------

void SwgCuiAuctionListPane::startSale () const
{
	CuiMediatorFactory::deactivateInWorkspace (CuiMediatorTypes::WS_AuctionSell);
	AuctionManagerClient::setItemToSellFromStock (NetworkId::cms_invalid);
	CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_AuctionSell);
}

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::retrieve (const NetworkIdVector & auctionIds) const
{
	for (NetworkIdVector::const_iterator it = auctionIds.begin (); it != auctionIds.end (); ++it)
	{
		const NetworkId & id = *it;
		retrieve (id);
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::retrieve (const NetworkId & auctionId) const
{
	if (!auctionId.isValid ())
		return;

	if (!AuctionManagerClient::isItemRetrievable (auctionId))
		return;

	CuiAuctionManager::handleItemRetrieve (auctionId);
}

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::withdraw (const NetworkIdVector & auctionIds) const
{
	for (NetworkIdVector::const_iterator it = auctionIds.begin (); it != auctionIds.end (); ++it)
	{
		const NetworkId & id = *it;
		withdraw (id);
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::withdraw (const NetworkId & auctionId) const
{
	if (!auctionId.isValid ())
		return;

	if (!AuctionManagerClient::isItemWithdrawable (auctionId))
		return;

	CuiAuctionManager::handleWithdraw (auctionId, false);
}

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::acceptBid (const NetworkId & auctionId)
{
	if (!auctionId.isValid ())
		return;

	CuiAuctionManager::handleAcceptBid (auctionId, false);
}

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::enterBid () const
{
	const NetworkId & auctionId = findSelectedAuction (true);	
	enterBid (auctionId);
}

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::enterBid (const NetworkId & auctionId) const
{
	if (!auctionId.isValid ())
		return;

	const AuctionManagerClientData * data = AuctionManagerClient::findAuction (auctionId);
	if (!data)
		return;

	AuctionManagerClient::setAuctionToBidOn (auctionId);

	const bool isCommodity = AuctionManagerClient::isAtCommodityMarket ();

	if ((data->header.buyNowPrice > 0) || !isCommodity)
		CuiMediatorFactory::activateInWorkspace  (CuiMediatorTypes::WS_AuctionBuy);
	else
		CuiMediatorFactory::activateInWorkspace  (CuiMediatorTypes::WS_AuctionBid);
}

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::setPrice (const NetworkId & auctionId) const
{
	if (!auctionId.isValid ())
		return;

	AuctionManagerClient::setAuctionToSetPriceOn (auctionId);
	CuiMediatorFactory::activateInWorkspace   (CuiMediatorTypes::WS_VendorSetPrice);
}

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::reject (const NetworkId & auctionId) const
{
	if (!auctionId.isValid ())
		return;

	CuiAuctionManager::handleReject (auctionId, false);
}

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::createWaypoint(const NetworkId & auctionId,  bool includePriceInWaypointName) const
{
	if (!auctionId.isValid())
	{
		return;
	}

	AuctionManagerClientData const * const data = AuctionManagerClient::findAuction (auctionId);
	if (data == 0)
	{
		return;
	}

	Auction::ItemDataHeader const & dataHeader = data->header;

	std::string const & location = dataHeader.location;

	std::string planet;
	std::string region;
	std::string name;
	NetworkId id;
	int x = 0;
	int z = 0;
	AuctionManager::separateLocation(location, planet, region, name, id, x, z);

	Vector const position_w(static_cast<float>(x), 0.0f, static_cast<float>(z));

	Unicode::String wpName = StringId("ui_auc", "waypoint_to_vendor_name").localize();
	wpName += Unicode::narrowToWide(" (");

	if (includePriceInWaypointName)
	{
		char buffer[64];
		snprintf(buffer, sizeof(buffer)-1, "%d cr", dataHeader.buyNowPrice);
		buffer[sizeof(buffer)-1] = '\0';
		wpName += UIUtils::FormatDelimitedInteger(Unicode::narrowToWide(buffer));

		wpName += Unicode::narrowToWide(", ");
	}

	wpName += dataHeader.itemName;
	wpName += Unicode::narrowToWide(")");

	ClientWaypointObject::requestWaypoint(wpName, planet, position_w, static_cast<uint8>(Waypoint::Green));
}

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::sellFromStock (const NetworkIdVector & auctionIds) const
{
	if (auctionIds.empty ())
		return;

	CuiMediatorFactory::deactivateInWorkspace (CuiMediatorTypes::WS_AuctionSell);
	AuctionManagerClient::setItemsToSellFromStock (auctionIds);
	CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_AuctionSell);
}

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::relistFromStock (const NetworkIdVector & auctionIds) const
{
	if (auctionIds.empty ())
		return;

	CuiAuctionManager::handleVendorRelist(auctionIds);
}

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::onTraverseAuctionToView (const AuctionManagerClient::Messages::TraverseAuctionToView::Payload & forward)
{
	NOT_NULL (m_view);
	m_view->traverseAuctionToView (forward);
	showDetails (findSelectedAuction (true));
}

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::onViewDoubleClick         (const View & view)
{
	if (view.getType () == m_type)
		showDetails (findSelectedAuction (true));
}  //lint !e1762 // stfu

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::onViewSelectionChanged (const View & view)
{
	if (view.getType () == m_type)
	{
		updateButtonStates ();
	}
}  //lint !e1762 // stfu

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::onViewContextRequest (const View::Messages::ContextRequest::Payload & payload)
{
	const UIPoint & pt                  = payload.second.second;
	const NetworkId & selectedAuctionId = payload.second.first;
	
	if (selectedAuctionId.isValid ())
	{
		const AuctionManagerClientData * const data = AuctionManagerClient::findAuction (selectedAuctionId);
		
		if (data)
		{
			UIPopupMenu * const pop = UIPopupMenu::CreatePopupMenu (getPage ());
			if (pop)
			{				
				pop->AddItem (PopupData::details, CuiStringIdsAuction::details.localize ());
				
				if (m_buttonBid)
				{
					if (data->header.buyNowPrice > 0)
						pop->AddItem (PopupData::buy, CuiStringIdsAuction::buy.localize ());
					else
						pop->AddItem (PopupData::place_bid, CuiStringIdsAuction::place_bid.localize ());
				}
				
				if (m_buttonSell)
					pop->AddItem (PopupData::sell_new, CuiStringIdsAuction::sell_new.localize ());

				if (m_buttonRelistFromStock)
					pop->AddItem (PopupData::relist_from_stock, CuiStringIdsAuction::relist_from_stock.localize ());
				
				if (m_buttonSellFromStock)
					pop->AddItem (PopupData::sell_from_stock, CuiStringIdsAuction::sell_from_stock.localize ());
				
				if (m_buttonOffer)
					pop->AddItem (PopupData::offer, CuiStringIdsAuction::offer.localize ());
				
				if (m_buttonReject)
					pop->AddItem (PopupData::reject, CuiStringIdsAuction::reject.localize ());

				if (m_buttonCreateWaypoint)
					pop->AddItem (PopupData::createWaypoint, CuiStringIdsAuction::waypoint_to_vendor.localize ());

				if (m_buttonWithdraw && isSelectionWithdrawable ())
					pop->AddItem (PopupData::withdraw, CuiStringIdsAuction::withdraw.localize ());
				
				if (m_buttonAccept && isSelectionAcceptable ())
					pop->AddItem (PopupData::accept_bid, CuiStringIdsAuction::accept_bid.localize ());
				
				if (m_buttonRetrieve && isSelectionRetrievable ())
					pop->AddItem (PopupData::retrieve, CuiStringIdsAuction::retrieve.localize ());
				
				pop->SetLocation (pt);				
				pop->AddCallback (this);

				setIdsForPopup (*pop, findSelectedAuctions (false));

				UIManager::gUIManager ().PushContextWidget (*pop);
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::updateTextListStatus      ()
{
	if (!m_textListStatus && !m_textPaging)
		return;
	
	int currentIndex = 0;
	bool hasMoreAuctions = false;
	int pageSize = 0;
	AuctionManagerClient::DataVector dv;
	AuctionManagerClient::getAuctions  (static_cast<AuctionManagerClient::AuctionListType>(m_auctionListType), dv, currentIndex, hasMoreAuctions, pageSize);
			
	bool ready = true;
	
	if (!AuctionManagerClient::isListRequestOutstanding (static_cast<AuctionManagerClient::AuctionListType>(m_auctionListType)))
	{
		if (!AuctionManagerClient::isListValid (static_cast<AuctionManagerClient::AuctionListType>(m_auctionListType)))
		{
			if (m_textListStatus)
			{
				if (m_filter)
					m_textListStatus->SetLocalText (CuiStringIdsAuction::list_test_status_no_list_filter.localize ());
				else
					m_textListStatus->SetLocalText (CuiStringIdsAuction::list_test_status_no_list.localize ());
			}
			ready = false;
		}
		
		if (ready)
		{
			if (dv.empty ())
			{
				if (m_textListStatus)
				{
					m_textListStatus->SetLocalText (CuiStringIdsAuction::list_test_status_no_entries.localize ());
				}
				ready = false;
			}
		}
	}
	
	if (m_textListStatus)
		m_textListStatus->SetVisible (!ready);

	const int auctionCount = static_cast<int>(dv.size ());

	if (m_textPaging)
	{
		if (!ready)
			m_textPaging->SetVisible (false);
		else
		{
			m_textPaging->SetVisible (true);
			char buf [128];
			const size_t buf_size = sizeof (buf);

			snprintf (buf, buf_size, "%d-%d", currentIndex + 1, currentIndex + auctionCount);
			m_textPaging->SetLocalText (CuiStringIdsAuction::page_text_prefix.localize () + Unicode::narrowToWide (buf));
		}
	}

	if (m_buttonPagePrev)
		m_buttonPagePrev->SetVisible (currentIndex > 0);

	if (m_buttonPageNext)
		m_buttonPageNext->SetVisible (hasMoreAuctions);

}

//----------------------------------------------------------------------

bool SwgCuiAuctionListPane::isSelectionWithdrawable () const
{
	const NetworkIdVector & niv = findSelectedAuctions (false);
	for (NetworkIdVector::const_iterator it = niv.begin (); it != niv.end (); ++it)
	{
		if (AuctionManagerClient::isItemWithdrawable (*it))
			return true;
	}

	return false;
}

//----------------------------------------------------------------------

bool SwgCuiAuctionListPane::isSelectionAcceptable () const
{
	const NetworkIdVector & niv = findSelectedAuctions (false);
	for (NetworkIdVector::const_iterator it = niv.begin (); it != niv.end (); ++it)
	{
		if (AuctionManagerClient::isItemAcceptable (*it))
			return true;
	}

	return false;
}

//----------------------------------------------------------------------

bool SwgCuiAuctionListPane::isSelectionRetrievable () const
{
	const NetworkIdVector & niv = findSelectedAuctions (false);
	for (NetworkIdVector::const_iterator it = niv.begin (); it != niv.end (); ++it)
	{
		if (AuctionManagerClient::isItemRetrievable (*it))
			return true;
	}

	return false;
}

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::updateButtonStates ()
{
	const NetworkId & selectedAuctionId = findSelectedAuction  (false);

	const AuctionManagerClientData * const data = AuctionManagerClient::findAuction (selectedAuctionId);

	if (m_buttonWithdraw)
		m_buttonWithdraw->SetEnabled (data && isSelectionWithdrawable ());

	if (m_buttonAccept)
		m_buttonAccept->SetEnabled   (data && isSelectionAcceptable ());

	if (m_buttonRetrieve)
		m_buttonRetrieve->SetEnabled   (data && isSelectionRetrievable ());

	if (m_buttonDetails)
		m_buttonDetails->SetEnabled       (data);

	if (m_buttonSellFromStock)
		m_buttonSellFromStock->SetEnabled (data);

	if (m_buttonRelistFromStock)
		m_buttonRelistFromStock->SetEnabled (data);

	if (m_buttonChangePrice)
		m_buttonChangePrice->SetEnabled   (data);

	if (m_buttonReject)
		m_buttonReject->SetEnabled        (data);

	if (m_buttonCreateWaypoint)
		m_buttonCreateWaypoint->SetEnabled(data);
}

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::OnButtonPressed (UIWidget * context)
{
	if (context == m_buttonBid)
		enterBid ();
	else if (context == m_buttonDetails)
		showDetails (findSelectedAuction (true));
	else if (context == m_buttonWithdraw)
		withdraw (findSelectedAuctions (true));
	else if (context == m_buttonAccept)
		acceptBid (findSelectedAuction (true));
	else if (context == m_buttonRetrieve)
		retrieve (findSelectedAuctions (true));
	else if (context == m_buttonSell)
		startSale ();
	else if (context == m_buttonSellFromStock)
		sellFromStock (findSelectedAuctions (true));
	else if (context == m_buttonRelistFromStock)
	{
		relistFromStock (findSelectedAuctions (true));

		// unselected all items to prevent player from spamming the relist item
		// button and sending unnecessary/duplicate request for the same items
		m_view->clearAllSelection();
		updateButtonStates();
	}
	else if (context == m_buttonOffer)
		startSale ();
	else if (context == m_buttonChangePrice)
		setPrice (findSelectedAuction (true));
	else if (context == m_buttonReject)
		reject (findSelectedAuction (true));
	else if (context == m_buttonCreateWaypoint)
		createWaypoint(findSelectedAuction (true), (m_filter != NULL)); // if from Vendor Location tab, include price in waypoint name
	else if (context == m_buttonPagePrev || context == m_buttonPageNext)
	{
		int currentIndex = 0;
		bool hasMoreAuctions = false;
		int pageSize = 0;
		AuctionManagerClient::DataVector dv;
		AuctionManagerClient::getAuctions  (static_cast<AuctionManagerClient::AuctionListType>(m_auctionListType), dv, currentIndex, hasMoreAuctions, pageSize);
	
		if (context == m_buttonPagePrev)
			requestUpdate (false, -(pageSize));
		else if (context == m_buttonPageNext)
			requestUpdate (false, pageSize);
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::OnPopupMenuSelection (UIWidget * context)
{
	UIPopupMenu * const pop = NON_NULL (safe_cast<UIPopupMenu *>(context));
	if (pop)
	{
		const NetworkIdVector & niv = getIdsFromPopup (*pop);

		if (niv.empty ())
			return;

		const NetworkId & id = niv.back ();
		const AuctionManagerClientData * const data = AuctionManagerClient::findAuction (id);
		
		if (data)
		{
			const std::string & sel = pop->GetSelectedName ();
			if (sel == PopupData::details)
			{
				showDetails (id);
			}
			else if (sel == PopupData::buy)
			{
				enterBid (id);
			}
			else if (sel == PopupData::place_bid)
			{
				enterBid (id);
			}
			else if (sel == PopupData::sell_new)
			{
				startSale ();
			}
			else if (sel == PopupData::sell_from_stock)
			{
				sellFromStock (niv);
			}
			else if (sel == PopupData::relist_from_stock)
			{
				relistFromStock (niv);

				// unselected all items to prevent player from spamming the relist item
				// button and sending unnecessary/duplicate request for the same items
				m_view->clearAllSelection();
				updateButtonStates();
			}
			else if (sel == PopupData::offer)
			{
				startSale ();
			}
			else if (sel == PopupData::reject)
			{
				reject (id);
			}
			else if (sel == PopupData::createWaypoint)
			{
				createWaypoint(id, (m_filter != NULL)); // if from Vendor Location tab, include price in waypoint name
			}
			else if (sel == PopupData::withdraw)
			{
				withdraw (niv);
			}
			else if (sel == PopupData::accept_bid)
			{
				acceptBid (id);
			}
			else if (sel == PopupData::retrieve)
			{
				retrieve (niv);
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::update (float deltaTimeSecs)
{
	if (m_view)
		m_view->update (deltaTimeSecs);
}

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::saveSettings         () const
{
	CuiMediator::saveSettings ();

	if (m_filter)
	{
		const int filterType = m_filter->getSelectedObjectType ();
		CuiSettings::saveInteger (getMediatorDebugName (), Settings::filterType, filterType);
	}

	if(m_view)
		m_view->saveSettings(m_auctionListType);
}

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::loadSettings ()
{
	CuiMediator::loadSettings ();

	if (m_filter)
	{
		int filterType = 0;
		
		if (!CuiSettings::loadInteger (getMediatorDebugName (), Settings::filterType, filterType))
			return;

		m_filter->setSelectedObjectType (filterType, false);
	}

	if(m_view)
		m_view->loadSettings(m_auctionListType);
}

//----------------------------------------------------------------------

void SwgCuiAuctionListPane::onListViewUpdated ()
{
	updateTextListStatus ();
}

//======================================================================

