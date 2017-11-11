//======================================================================
//
// SwgCuiAuctionBuy.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgclientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAuctionBuy.h"

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

SwgCuiAuctionBuy::SwgCuiAuctionBuy (UIPage & page) :
CuiMediator       ("SwgCuiAuctionBuy", page),
UIEventCallback   (),
m_textItemName    (0),
m_textInstant     (0),
m_buttonCancel    (0),
m_buttonOk        (0),
m_highBid         (0),
m_callback        (new MessageDispatch::Callback),
m_messageBox      (0)
{
	setState    (MS_closeable);
	setState    (MS_closeDeactivates);

	getCodeDataObject (TUIText,     m_textItemName,          "textItemName");
	getCodeDataObject (TUIText,     m_textInstant,           "textInstant");
	getCodeDataObject (TUIButton,   m_buttonCancel,          "buttonCancel");
	getCodeDataObject (TUIButton,   m_buttonOk,              "buttonOk");
	
	m_textInstant->SetPreLocalized  (true);
	m_textItemName->SetPreLocalized (true);

	registerMediatorObject (*m_textItemName,    true);
	registerMediatorObject (*m_textInstant,     true);
	registerMediatorObject (*m_buttonCancel,    true);
	registerMediatorObject (*m_buttonOk,        true);
}

//----------------------------------------------------------------------

SwgCuiAuctionBuy::~SwgCuiAuctionBuy ()
{
	delete m_callback;
	m_callback = 0;
}

//----------------------------------------------------------------------

void SwgCuiAuctionBuy::performActivate   ()
{
	float range = 0.0f;
	if (RadialMenuManager::getRangeForMenuType (Cui::MenuInfoTypes::TERMINAL_AUCTION_USE, range))
	{
		setAssociatedObjectId (AuctionManagerClient::getMarketObjectId ());
		setMaxRangeFromObject (range);
	}

	CuiManager::requestPointer (true);

	m_callback->connect (*this, &SwgCuiAuctionBuy::onAuctionToBidOnChanged, static_cast<AuctionManagerClient::Messages::AuctionToBidOnChanged *> (0));
	m_callback->connect (*this, &SwgCuiAuctionBuy::onAuctionBidResponse,    static_cast<AuctionManagerClient::Messages::BidResponse *> (0));
	onAuctionToBidOnChanged (AuctionManagerClient::getAuctionToBidOn ());

	setIsUpdating (true);
}

//----------------------------------------------------------------------

void SwgCuiAuctionBuy::performDeactivate ()
{
	CuiManager::requestPointer (false);

	m_callback->disconnect (*this, &SwgCuiAuctionBuy::onAuctionToBidOnChanged, static_cast<AuctionManagerClient::Messages::AuctionToBidOnChanged *> (0));
	m_callback->disconnect (*this, &SwgCuiAuctionBuy::onAuctionBidResponse,    static_cast<AuctionManagerClient::Messages::BidResponse *> (0));

	if (m_messageBox)
		m_messageBox->closeMessageBox ();

	m_messageBox = 0;

	setIsUpdating (false);
}

//----------------------------------------------------------------------

void SwgCuiAuctionBuy::OnButtonPressed   (UIWidget *context)
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

void SwgCuiAuctionBuy::ok ()
{
	int playerCash = 0;
	int playerBank = 0;
	
	PlayerMoneyManagerClient::getPlayerMoney (playerCash, playerBank);
	
	const NetworkId & auctionId = AuctionManagerClient::getAuctionToBidOn ();
	const AuctionManagerClientData * data = AuctionManagerClient::findAuction (auctionId);
	
	if (!data)
	{
		CuiMessageBox::createInfoBox (CuiStringIdsAuction::err_bid_invalid_item.localize ());
		deactivate ();
		return;
	}
	
	if (m_highBid > (playerBank + playerCash))
	{
		CuiMessageBox::createInfoBox (CuiStringIdsAuction::err_buy_not_enough_money.localize ());
		return;
	}
	
	AuctionManagerClient::makeBid (data->header.itemId, m_highBid, m_highBid, true);
	
	if (m_messageBox)
		m_messageBox->closeMessageBox ();
	
	m_messageBox = CuiMessageBox::createMessageBox (CuiStringIdsAuction::waiting_buy.localize ());
	
	m_callback->connect (m_messageBox->getTransceiverClosed (), *this, &SwgCuiAuctionBuy::onMessageBoxClosed);
}

//----------------------------------------------------------------------

void SwgCuiAuctionBuy::onAuctionToBidOnChanged (const AuctionManagerClient::Messages::AuctionToBidOnChanged::Payload & )
{
	const NetworkId & auctionId = AuctionManagerClient::getAuctionToBidOn ();
	const AuctionManagerClientData * data = AuctionManagerClient::findAuction (auctionId);
	
	if (data)
	{

		if (data->header.buyNowPrice <= 0)
		{
			deactivate ();
			return;
		}

		Unicode::String nameStr;
		data->constructLocalizedName (nameStr);
		m_textItemName->SetLocalText (nameStr);
		
		UIBaseObject * const parent = getPage ().GetParent ();
		if (parent)
			parent->MoveChild (&getPage (), UIBaseObject::Top);
		
		
		m_highBid = data->header.highBid;
		Unicode::String highBidStr;
		UIUtils::FormatInteger (highBidStr, m_highBid);
		highBidStr = UIUtils::FormatDelimitedInteger(highBidStr);
				
		m_textInstant->SetLocalText (highBidStr);
		m_textInstant->SetVisible   (true);

	}
	else
	{
		WARNING (true, ("bad bid id"));
		deactivate ();
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionBuy::onMessageBoxClosed      (const CuiMessageBox & box)
{
	if (&box == m_messageBox)
		m_messageBox = 0;

	getPage().SetFocus();
}

//----------------------------------------------------------------------

void SwgCuiAuctionBuy::onAuctionBidResponse    (const AuctionManagerClient::Messages::BidResponse::Payload & result)
{
	if (m_messageBox)
		m_messageBox->closeMessageBox ();

	CuiMessageBox::createInfoBox (result.second.second);

	if (result.second.first == 0)
		deactivate ();
}

//----------------------------------------------------------------------

void SwgCuiAuctionBuy::update(float deltaTimeSecs)
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
			m_callback->connect(messageBox->getTransceiverClosed (), *this, &SwgCuiAuctionBuy::onMessageBoxClosed);
		}
	}
}

//----------------------------------------------------------------------

bool SwgCuiAuctionBuy::close()
{
	CuiAuctionManager::setLastContainerItemId(NetworkId::cms_invalid);

	return CuiMediator::close();
}

//======================================================================
