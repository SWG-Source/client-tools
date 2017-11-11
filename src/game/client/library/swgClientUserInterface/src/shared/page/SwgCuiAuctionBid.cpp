//======================================================================
//
// SwgCuiAuctionBid.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgclientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAuctionBid.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIPage.h"
#include "UIText.h"
#include "UITextbox.h"
#include "UIUtils.h"
#include "clientGame/AuctionManagerClient.h"
#include "clientGame/AuctionManagerClientData.h"
#include "clientGame/PlayerMoneyManagerClient.h"
#include "clientUserInterface/CuiAuctionManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMenuInfoTypes.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiMoney.h"
#include "clientUserInterface/CuiStringIdsAuction.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "sharedGame/RadialMenuManager.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/CachedNetworkId.h"


//----------------------------------------------------------------------

SwgCuiAuctionBid::SwgCuiAuctionBid (UIPage & page) :
CuiMediator       ("SwgCuiAuctionBid", page),
UIEventCallback   (),
m_textItemName    (0),
m_textHighBid     (0),
m_textboxBid      (0),
m_textboxProxyBid (0),
m_buttonCancel    (0),
m_buttonOk        (0),
m_ignoreTextbox   (false),
m_highBid         (0),
m_callback        (new MessageDispatch::Callback),
m_messageBox      (0)
{
	setState    (MS_closeable);
	setState    (MS_closeDeactivates);

	getCodeDataObject (TUIText,     m_textItemName,          "textItemName");
	getCodeDataObject (TUIText,     m_textHighBid,           "textHighBid");
	getCodeDataObject (TUITextbox,  m_textboxBid,            "textboxBid");
	getCodeDataObject (TUITextbox,  m_textboxProxyBid,       "textboxProxyBid");	
	getCodeDataObject (TUIButton,   m_buttonCancel,          "buttonCancel");
	getCodeDataObject (TUIButton,   m_buttonOk,              "buttonOk");
	
	m_textHighBid->SetPreLocalized (true);
	m_textItemName->SetPreLocalized (true);

	registerMediatorObject (*m_textItemName,    true);
	registerMediatorObject (*m_textHighBid,     true);
	registerMediatorObject (*m_textboxBid,      true);
	registerMediatorObject (*m_textboxProxyBid, true);
	registerMediatorObject (*m_buttonCancel,    true);
	registerMediatorObject (*m_buttonOk,        true);
}

//----------------------------------------------------------------------

SwgCuiAuctionBid::~SwgCuiAuctionBid ()
{
	delete m_callback;
	m_callback = 0;
}

//----------------------------------------------------------------------

void SwgCuiAuctionBid::performActivate   ()
{
	float range = 0.0f;
	if (RadialMenuManager::getRangeForMenuType (Cui::MenuInfoTypes::TERMINAL_AUCTION_USE, range))
	{
		setAssociatedObjectId (AuctionManagerClient::getMarketObjectId ());
		setMaxRangeFromObject (range);
	}

	CuiManager::requestPointer (true);

	m_callback->connect (*this, &SwgCuiAuctionBid::onAuctionToBidOnChanged, static_cast<AuctionManagerClient::Messages::AuctionToBidOnChanged *> (0));
	m_callback->connect (*this, &SwgCuiAuctionBid::onAuctionBidResponse,    static_cast<AuctionManagerClient::Messages::BidResponse *> (0));
	onAuctionToBidOnChanged (AuctionManagerClient::getAuctionToBidOn ());

	setIsUpdating (true);
}

//----------------------------------------------------------------------

void SwgCuiAuctionBid::performDeactivate ()
{
	CuiManager::requestPointer (false);

	m_callback->disconnect (*this, &SwgCuiAuctionBid::onAuctionToBidOnChanged, static_cast<AuctionManagerClient::Messages::AuctionToBidOnChanged *> (0));
	m_callback->disconnect (*this, &SwgCuiAuctionBid::onAuctionBidResponse,    static_cast<AuctionManagerClient::Messages::BidResponse *> (0));

	if (m_messageBox)
		m_messageBox->closeMessageBox ();

	m_messageBox = 0;

	setIsUpdating (false);
}

//----------------------------------------------------------------------

void SwgCuiAuctionBid::OnTextboxChanged  (UIWidget *context)
{
	if (context == m_textboxBid)
	{
		if (m_ignoreTextbox)
			return;
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionBid::OnButtonPressed   (UIWidget *context)
{
	if (context == m_buttonOk)
	{
		ok ();
	}
	else if (context == m_buttonCancel)
	{
		closeThroughWorkspace ();
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionBid::ok ()
{
	const int value = m_textboxBid->GetNumericIntegerValue ();
	const int proxyBid = m_textboxProxyBid->GetNumericIntegerValue ();

	if (value <= m_highBid)
	{
		CuiMessageBox::createInfoBox (CuiStringIdsAuction::err_bid_invalid.localize ());
		return;
	}

	const NetworkId & auctionId = AuctionManagerClient::getAuctionToBidOn ();
	const AuctionManagerClientData * data = AuctionManagerClient::findAuction (auctionId);

	if (!data)
	{
		CuiMessageBox::createInfoBox (CuiStringIdsAuction::err_bid_invalid_item.localize ());
		deactivate ();
		return;
	}

	int playerCash = 0;
	int playerBank = 0;
	PlayerMoneyManagerClient::getPlayerMoney (playerCash, playerBank);

	if (proxyBid > (playerBank + playerCash))
	{
		CuiMessageBox::createInfoBox (CuiStringIdsAuction::err_bid_not_enough_money.localize ());
		return;
	}

	AuctionManagerClient::makeBid (data->header.itemId, value, proxyBid, false);

	if (m_messageBox)
		m_messageBox->closeMessageBox ();

	m_messageBox = CuiMessageBox::createMessageBox (CuiStringIdsAuction::waiting_bid.localize ());

	m_callback->connect (m_messageBox->getTransceiverClosed (), *this, &SwgCuiAuctionBid::onMessageBoxClosed);
}

//----------------------------------------------------------------------

void SwgCuiAuctionBid::onAuctionToBidOnChanged (const AuctionManagerClient::Messages::AuctionToBidOnChanged::Payload & )
{
	const NetworkId & auctionId = AuctionManagerClient::getAuctionToBidOn ();
	const AuctionManagerClientData * data = AuctionManagerClient::findAuction (auctionId);

	if (data)
	{
		if (data->header.buyNowPrice > 0)
		{
			deactivate ();
			return;
		}

		m_highBid = data->header.highBid;
		Unicode::String str;
		UIUtils::FormatInteger (str, m_highBid);
		str = UIUtils::FormatDelimitedInteger(str);
		m_textHighBid->SetLocalText (str);

		const int bid = m_highBid + std::max (10, m_highBid * 10 / 100);
		UIUtils::FormatInteger (str, bid);
		m_textboxBid->SetLocalText (str);
		m_textboxBid->MoveCaratToEndOfLine ();

		const int proxyBid = m_highBid + std::max (20, m_highBid * 20 / 100);
		UIUtils::FormatInteger (str, proxyBid);
		m_textboxProxyBid->SetLocalText (str);
		m_textboxProxyBid->MoveCaratToEndOfLine ();

		data->constructLocalizedName (str);
		m_textItemName->SetLocalText (str);

		UIBaseObject * const parent = getPage ().GetParent ();
		if (parent)
			parent->MoveChild (&getPage (), UIBaseObject::Top);

		m_textboxBid->SetFocus ();
	}
	else
	{
		WARNING (true, ("bad bid id"));
		deactivate ();
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionBid::onMessageBoxClosed      (const CuiMessageBox & box)
{
	if (&box == m_messageBox)
		m_messageBox = 0;

	getPage().SetFocus();
}

//----------------------------------------------------------------------

void SwgCuiAuctionBid::onAuctionBidResponse    (const AuctionManagerClient::Messages::BidResponse::Payload & result)
{
	if (m_messageBox)
		m_messageBox->closeMessageBox ();

	CuiMessageBox::createInfoBox (result.second.second);

	if (result.second.first == 0)
		deactivate ();
}

//----------------------------------------------------------------------

void SwgCuiAuctionBid::update(float deltaTimeSecs)
{
	UNREF(deltaTimeSecs);

	NetworkId const &auctionId = AuctionManagerClient::getAuctionToBidOn();

	if (CuiAuctionManager::getLastContainerItemId() != auctionId)
	{
		CuiAuctionManager::setLastContainerItemId(auctionId);

		// If this is a container item, warn the player about the contents
		// not being viewable before the purchase

		AuctionManagerClientData const * const auctionData = AuctionManagerClient::findAuction(auctionId);

		if (   (auctionData != NULL)
			&& ((auctionData->header.itemType == SharedObjectTemplate::GOT_misc_container)
			||  (auctionData->header.itemType == SharedObjectTemplate::GOT_misc_container_wearable)))
		{
			CuiStringVariablesData data;
			data.targetName = auctionData->header.itemName;
			Unicode::String confirmationString;
			CuiStringVariablesManager::process(CuiStringIdsAuction::container_warning_prose, data, confirmationString);

			CuiMessageBox *messageBox = CuiMessageBox::createInfoBox(confirmationString);
			m_callback->connect(messageBox->getTransceiverClosed (), *this, &SwgCuiAuctionBid::onMessageBoxClosed);
		}
	}
}

//----------------------------------------------------------------------

bool SwgCuiAuctionBid::close()
{
	CuiAuctionManager::setLastContainerItemId(NetworkId::cms_invalid);

	return CuiMediator::close();
}

//======================================================================
