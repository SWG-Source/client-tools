//======================================================================
//
// SwgCuiAuctionDetails.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgclientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAuctionDetails.h"

#include "UIButton.h"
#include "UIComposite.h"
#include "UIData.h"
#include "UIPage.h"
#include "UIText.h"
#include "UIUtils.h"
#include "clientGame/AuctionManagerClient.h"
#include "clientGame/AuctionManagerClientData.h"
#include "clientGame/ClientTextManager.h"
#include "clientGame/Game.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientUserInterface/CuiAuctionManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMenuInfoTypes.h"
#include "clientUserInterface/CuiMoney.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiStringIdsAuction.h"
#include "clientUserInterface/CuiUtils.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/RadialMenuManager.h"
#include "sharedGame/ResourceClassObject.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/Universe.h"
#include "sharedMath/PackedRgb.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/CachedNetworkId.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

//----------------------------------------------------------------------

SwgCuiAuctionDetails::SwgCuiAuctionDetails (UIPage & page) :
CuiMediator             ("SwgCuiAuctionDetails", page),
UIEventCallback         (),
m_buttonExit            (0),
m_buttonBid             (0),
m_buttonRetrieve        (0),
m_buttonSell            (0),
m_buttonWithdraw        (0),
m_buttonAccept          (0),
m_buttonNext            (0),
m_buttonPrev            (0),
m_compositeText         (0),
m_compositeDesc         (0),
m_textPrice             (0),
m_labelPrice            (0),
m_textItemName          (0),
m_textLocation          (0),
m_textSeller            (0),
m_textTime              (0),
m_textType              (0),
m_textDesc              (0),
m_textAttribs           (0),
m_viewer                (0),
m_callback              (new MessageDispatch::Callback),
m_itemId                (),
m_lastSecsRemaining     (-1),
m_elapsedTime           (0.0f),
m_buyButtonEnabled(true)
{
	setState    (MS_closeable);
	setState    (MS_closeDeactivates);
	
	getCodeDataObject (TUIButton,    m_buttonExit,        "buttonExit");
	getCodeDataObject (TUIButton,    m_buttonBid,         "buttonBid");
	getCodeDataObject (TUIButton,    m_buttonRetrieve,    "buttonRetrieve");
	getCodeDataObject (TUIButton,    m_buttonSell,        "buttonSell");
	getCodeDataObject (TUIButton,    m_buttonWithdraw,    "buttonWithdraw");
	getCodeDataObject (TUIButton,    m_buttonAccept,      "buttonAccept");
	getCodeDataObject (TUIButton,    m_buttonNext,        "buttonNext");
	getCodeDataObject (TUIButton,    m_buttonPrev,        "buttonPrev");
	getCodeDataObject (TUIComposite, m_compositeText,     "compositeText");
	getCodeDataObject (TUIComposite, m_compositeDesc,     "compositeDesc");
	getCodeDataObject (TUIText,      m_textPrice,         "textPrice");
	getCodeDataObject (TUIText,      m_labelPrice,        "labelPrice");
	getCodeDataObject (TUIText,      m_textItemName,      "textItemName");
	getCodeDataObject (TUIText,      m_textLocation,      "textLocation");
	getCodeDataObject (TUIText,      m_textSeller,        "textSeller");
	getCodeDataObject (TUIText,      m_textTime,          "textTime");
	getCodeDataObject (TUIText,      m_textType,          "textType");
	getCodeDataObject (TUIText,      m_textDesc,          "textDesc");
	getCodeDataObject (TUIText,      m_textAttribs,       "textAttribs");

	{
		UIWidget * wid = 0;
		getCodeDataObject (TUIWidget,      wid,              "viewer");
		m_viewer = safe_cast<CuiWidget3dObjectListViewer *>(wid);
	}

	registerMediatorObject (*m_buttonExit,       true);
	registerMediatorObject (*m_buttonBid,        true);
	registerMediatorObject (*m_buttonRetrieve,   true);
	registerMediatorObject (*m_buttonSell,       true);
	registerMediatorObject (*m_buttonWithdraw,   true);
	registerMediatorObject (*m_buttonAccept,     true);
	registerMediatorObject (*m_buttonNext,       true);
	registerMediatorObject (*m_buttonPrev,       true);
	
	m_textPrice->SetPreLocalized       (true);
	m_labelPrice->SetPreLocalized      (true);
	m_textItemName->SetPreLocalized    (true);
	m_textLocation->SetPreLocalized    (true);
	m_textSeller->SetPreLocalized      (true);
	m_textTime->SetPreLocalized        (true);
	m_textType->SetPreLocalized        (true);
	m_textDesc->SetPreLocalized        (true);
	m_textAttribs->SetPreLocalized     (true);
}

//----------------------------------------------------------------------

SwgCuiAuctionDetails::~SwgCuiAuctionDetails ()
{
	delete m_callback;
	m_callback = 0;
}

//----------------------------------------------------------------------

void SwgCuiAuctionDetails::performActivate   ()
{
	float range = 0.0f;
	if (RadialMenuManager::getRangeForMenuType (Cui::MenuInfoTypes::TERMINAL_AUCTION_USE, range))
	{
		setAssociatedObjectId (AuctionManagerClient::getMarketObjectId ());
		setMaxRangeFromObject (range);
	}

	CuiManager::requestPointer (true);

	m_viewer->setPaused (false);

	m_callback->connect (*this, &SwgCuiAuctionDetails::onDetailsReceived,       static_cast<AuctionManagerClient::Messages::DetailsReceived*> (0));
	m_callback->connect (*this, &SwgCuiAuctionDetails::onAuctionToViewChanged, static_cast<AuctionManagerClient::Messages::AuctionToViewChanged *> (0));

	onAuctionToViewChanged (NetworkId::cms_invalid);

	setIsUpdating (true);
}

//----------------------------------------------------------------------

void SwgCuiAuctionDetails::performDeactivate ()
{
	CuiManager::requestPointer (false);

	m_viewer->setPaused (true);

	m_callback->disconnect (*this, &SwgCuiAuctionDetails::onAuctionToViewChanged, static_cast<AuctionManagerClient::Messages::AuctionToViewChanged *> (0));
	m_callback->disconnect (*this, &SwgCuiAuctionDetails::onDetailsReceived,       static_cast<AuctionManagerClient::Messages::DetailsReceived*> (0));

	m_itemId = NetworkId::cms_invalid;
	setIsUpdating  (false);
}

//----------------------------------------------------------------------

void SwgCuiAuctionDetails::setBuyButtonEnabled(bool const enabled)
{
	m_buyButtonEnabled = enabled;
}

//----------------------------------------------------------------------

void SwgCuiAuctionDetails::populate        (const Data & data)
{
	m_itemId = data.header.itemId;
	
	Unicode::String tmpStr;
	
	UIUtils::FormatInteger (tmpStr, static_cast<int>(data.header.highBid));
	tmpStr = UIUtils::FormatDelimitedInteger(tmpStr);
	m_textPrice->SetLocalText (tmpStr);
	
	const bool isRetrievable  = AuctionManagerClient::isItemRetrievable (data.header.itemId);
	const bool isWithdrawable = !isRetrievable && AuctionManagerClient::isItemWithdrawable (data.header.itemId);
	const bool isAcceptable   = isWithdrawable && AuctionManagerClient::isItemAcceptable (data.header.itemId);

	m_buttonRetrieve->SetVisible (isRetrievable);
	m_buttonWithdraw->SetVisible (isWithdrawable);
	m_buttonAccept->SetVisible   (isAcceptable);

	m_buttonBid->SetVisible (!isRetrievable && !isWithdrawable && !isAcceptable);

	const bool isCommodity = AuctionManagerClient::isAtCommodityMarket ();
	
	if (isCommodity)
	{
		m_buttonWithdraw->SetText (CuiStringIdsAuction::withdraw.localize ());
		m_buttonSell->SetVisible (false);
		if (data.header.buyNowPrice > 0)
		{
			m_textPrice->SetTextColor  (UIColor::green);
			m_labelPrice->SetLocalText (CuiStringIdsAuction::details_instant_sale.localize ());
			m_buttonBid->SetText       (CuiStringIdsAuction::buy.localize ());
		}
		else
		{
			m_textPrice->SetTextColor (UIColor::yellow);
			m_labelPrice->SetLocalText (CuiStringIdsAuction::details_high_bid.localize ());
			m_buttonBid->SetText       (CuiStringIdsAuction::place_bid.localize ());
		}

		updateTimeRemaining (data.header.timer);
	}
	else
	{
		m_buttonBid->SetText       (CuiStringIdsAuction::buy.localize ());
		m_textPrice->SetTextColor  (UIColor::green);
		m_labelPrice->SetLocalText (CuiStringIdsAuction::details_vendor_price.localize ());
		m_textTime->SetVisible     (false);
		
		if (AuctionManagerClient::isInStockroom (data.header.itemId))
		{
			m_buttonSell->SetVisible     (true);
		}
		else
		{
			if (m_buttonWithdraw->IsVisible ())
			{
				const Object * const player = Game::getPlayer ();
				if (player)
				{
					if (player->getNetworkId () == data.header.ownerId)
						m_buttonWithdraw->SetText    (CuiStringIdsAuction::withdraw.localize ());
					else
						m_buttonWithdraw->SetText    (CuiStringIdsAuction::reject.localize ());
				}
			}
			
			m_buttonSell->SetVisible     (false);
		}
	}
	
	data.constructLocalizedName (tmpStr);
	m_textItemName->SetLocalText (tmpStr);
	
	Unicode::String locationResult;
	AuctionManagerClient::localizeLocation (data.header.location, locationResult, true, false);
	m_textLocation->SetLocalText (locationResult);
	m_textSeller->SetLocalText   (Unicode::narrowToWide (data.header.ownerName));	
	
	m_compositeText->Pack ();

	populateDetails (data);
}

//----------------------------------------------------------------------

void SwgCuiAuctionDetails::populateDetails (const Data & data)
{
	m_viewer->clearObjects ();
	
	ResourceClassObject const * rco = NULL;
	if ((GameObjectTypes::isTypeOf(data.header.itemType, SharedObjectTemplate::GOT_resource_container)) && (data.header.resourceContainerClassCrc != 0))
		rco = Universe::getInstance().getResourceClassByNameCrc(static_cast<uint32>(data.header.resourceContainerClassCrc));

	Unicode::String typeStr;
	if (rco)
		typeStr = rco->getFriendlyName().localize();
	else
		typeStr = GameObjectTypes::getLocalizedName (static_cast<int>(data.header.itemType));

	m_textType->SetLocalText (typeStr);
	
	if (data.hasDetails)
	{
		Unicode::String str;

		static ObjectAttributeManager::AttributeVector mergedAttributeVector;
		mergedAttributeVector.clear ();
		
		ClientObject * const displayObj = AuctionManagerClient::getClientObjectForAuction (m_itemId);
		if (displayObj)
		{
			m_viewer->addObject (*displayObj);
			ObjectAttributeManager::populateMergedAttributes (*displayObj, mergedAttributeVector, data.details.propertyList);
		}
		else
			mergedAttributeVector = data.details.propertyList;

		ObjectAttributeManager::formatAttributes (mergedAttributeVector, str, NULL, NULL, false);

		if (CuiPreferences::getDebugExamine())
		{
			std::string debugString;
			std::string yellowColorCode(Unicode::wideToNarrow(ClientTextManager::getColorCode(PackedRgb::solidYellow)));
			std::string resetColorCode(Unicode::wideToNarrow(ClientTextManager::getResetTagCode()));
			char text[256];

			snprintf(text, sizeof(text), "%soid: %s%s\n", yellowColorCode.c_str(), resetColorCode.c_str(), data.header.itemId.getValueString().c_str());
			debugString += text;

			snprintf(text, sizeof(text), "%slocation oid: %s%s\n", yellowColorCode.c_str(), resetColorCode.c_str(), data.locationId.getValueString().c_str());
			debugString += text;

			snprintf(text, sizeof(text), "%sowner oid: %s%s\n", yellowColorCode.c_str(), resetColorCode.c_str(), data.header.ownerId.getValueString().c_str());
			debugString += text;

			snprintf(text, sizeof(text), "%shigh bidder oid: %s%s\n", yellowColorCode.c_str(), resetColorCode.c_str(), data.header.highBidderId.getValueString().c_str());
			debugString += text;

			Unicode::String debugWideString(Unicode::narrowToWide(debugString));

			str.insert(str.begin(), debugWideString.begin(), debugWideString.end());
		}

		m_textAttribs->SetLocalText (str);
		m_textDesc->SetLocalText    (data.details.userDescription);
		
		m_viewer->setCameraForceTarget (true);
		m_viewer->recomputeZoom        ();
		m_viewer->setCameraForceTarget (false);
	}
	else
	{
		m_textAttribs->SetLocalText (CuiStringIdsAuction::waiting_details.localize ());
		m_textDesc->SetLocalText    (Unicode::emptyString);
		
		AuctionManagerClient::requestDetails (data.header.itemId);
	}

	m_compositeDesc->Pack ();
}

//----------------------------------------------------------------------

void SwgCuiAuctionDetails::OnButtonPressed   (UIWidget * context)
{
	if (context == m_buttonExit)
	{
		closeThroughWorkspace ();
	}
	else if (context == m_buttonBid)
	{
		const AuctionManagerClientData * const data = AuctionManagerClient::findAuction (m_itemId);

		if (!data)
			return;

		AuctionManagerClient::setAuctionToBidOn (m_itemId);

		if (data->header.buyNowPrice > 0)
			CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_AuctionBuy);
		else
			CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_AuctionBid);
	}
	else if (context == m_buttonRetrieve)
	{
		CuiAuctionManager::handleItemRetrieve (m_itemId);
	}
	else if (context == m_buttonSell)
	{
		CuiMediatorFactory::deactivateInWorkspace (CuiMediatorTypes::WS_AuctionSell);
		AuctionManagerClient::setItemToSellFromStock (m_itemId);
		CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_AuctionSell);
		closeThroughWorkspace ();
	}
	else if (context == m_buttonNext)
	{
		AuctionManagerClient::traverseAuctionToView (true);
	}
	else if (context == m_buttonPrev)
	{
		AuctionManagerClient::traverseAuctionToView (false);
	}
	else if (context == m_buttonWithdraw)
	{
		if (!m_itemId.isValid ())
			return;
		
		CuiAuctionManager::handleWithdraw (m_itemId, false);
	}
	else if (context == m_buttonAccept)
	{
		if (!m_itemId.isValid ())
			return;

		CuiAuctionManager::handleAcceptBid (m_itemId, false);
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionDetails::onDetailsReceived (const Data & data)
{
	if (data.header.itemId == m_itemId)
	{
		//@todo: play 'data received' sound
		populateDetails (data);
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionDetails::onAuctionToViewChanged (const AuctionManagerClient::Messages::AuctionToViewChanged::Payload & )
{
	const NetworkId & auctionToView = AuctionManagerClient::getAuctionToView ();
	AuctionManagerClientData data;
	if (AuctionManagerClient::getAuctionData  (auctionToView, data))
	{
		UIBaseObject * const parent = getPage ().GetParent ();
		if (parent)
			parent->MoveChild (&getPage (), UIBaseObject::Top);

		populate (data);
	}
	else
		deactivate ();
}

//----------------------------------------------------------------------

void SwgCuiAuctionDetails::updateTimeRemaining (int secsRemaining)
{
	if (secsRemaining != m_lastSecsRemaining || !m_textType->IsVisible ())
	{
		static Unicode::String tmpStr;
		tmpStr.clear ();
		CuiUtils::FormatTimeDuration (tmpStr, secsRemaining, true, true, true, true);
		m_textTime->SetLocalText     (tmpStr);
		m_textTime->SetVisible       (true);
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionDetails::update                 (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	m_elapsedTime += deltaTimeSecs;

	if (m_elapsedTime >= 1.0f)
	{
		m_elapsedTime = 0.0f;

		if (m_itemId != NetworkId::cms_invalid)
		{
			const AuctionManagerClientData * data = AuctionManagerClient::findAuction (m_itemId);

			if (data)
				updateTimeRemaining (data->header.timer);
		}
	}

	if ((m_buttonBid != 0) && (m_buttonBid->IsVisible()) && (!m_buyButtonEnabled))
	{
		m_buttonBid->SetVisible(false);
	}
}

//======================================================================
