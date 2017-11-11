//======================================================================
//
// SwgCuiVendorSetPrice.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgclientUserInterface.h"
#include "swgClientUserInterface/SwgCuiVendorSetPrice.h"

//======================================================================

#include "UIButton.h"
#include "UIData.h"
#include "UIPage.h"
#include "UIText.h"
#include "UITextbox.h"
#include "UIUtils.h"
#include "clientGame/AuctionManagerClient.h"
#include "clientGame/AuctionManagerClientData.h"
#include "clientUserInterface/CuiMoney.h"
#include "sharedMessageDispatch/Transceiver.h"

//----------------------------------------------------------------------

SwgCuiVendorSetPrice::SwgCuiVendorSetPrice (UIPage & page) :
CuiMediator       ("SwgCuiVendorSetPrice", page),
UIEventCallback   (),
m_textItemName    (0),
m_textboxPrice    (0),
m_buttonCancel    (0),
m_buttonOk        (0),
m_callback        (new MessageDispatch::Callback)
{
	setState    (MS_closeable);
	setState    (MS_closeDeactivates);

	getCodeDataObject (TUIText,     m_textItemName,          "textItemName");
	getCodeDataObject (TUITextbox,  m_textboxPrice,          "textboxPrice");
	getCodeDataObject (TUIButton,   m_buttonCancel,          "buttonCancel");
	getCodeDataObject (TUIButton,   m_buttonOk,              "buttonOk");
	
	m_textItemName->SetPreLocalized (true);

	registerMediatorObject (*m_buttonCancel,    true);
	registerMediatorObject (*m_buttonOk,        true);
}

//----------------------------------------------------------------------

SwgCuiVendorSetPrice::~SwgCuiVendorSetPrice ()
{
	delete m_callback;
	m_callback = 0;
}

//----------------------------------------------------------------------

void SwgCuiVendorSetPrice::performActivate   ()
{
	getPage ().Center ();
	m_callback->connect (*this, &SwgCuiVendorSetPrice::onAuctionToSetPriceOnChanged, static_cast<AuctionManagerClient::Messages::AuctionToSetPriceOnChanged *> (0));
	onAuctionToSetPriceOnChanged (AuctionManagerClient::getAuctionToBidOn ());
}

//----------------------------------------------------------------------

void SwgCuiVendorSetPrice::performDeactivate ()
{
	m_callback->disconnect (*this, &SwgCuiVendorSetPrice::onAuctionToSetPriceOnChanged, static_cast<AuctionManagerClient::Messages::AuctionToSetPriceOnChanged *> (0));
}

//----------------------------------------------------------------------

void SwgCuiVendorSetPrice::OnButtonPressed   (UIWidget *context)
{
	if (context == m_buttonOk)
	{
		ok ();
		deactivate ();
	}
	else if (context == m_buttonCancel)
	{
		closeThroughWorkspace ();
	}
}

//----------------------------------------------------------------------

void SwgCuiVendorSetPrice::ok () const
{
}

//----------------------------------------------------------------------

void SwgCuiVendorSetPrice::onAuctionToSetPriceOnChanged (const AuctionManagerClient::Messages::AuctionToBidOnChanged::Payload & )
{
	const NetworkId & auctionId = AuctionManagerClient::getAuctionToSetPriceOn ();
	const AuctionManagerClientData * const data = AuctionManagerClient::findAuction (auctionId);
	
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
				
		Unicode::String priceStr;
		UIUtils::FormatInteger (priceStr, data->header.highBid);
		priceStr = UIUtils::FormatDelimitedInteger(priceStr);
				
		m_textboxPrice->SetLocalText (priceStr);
		m_textboxPrice->SetVisible   (true);
	}
	else
	{
		WARNING (true, ("bad bid id"));
		deactivate ();
	}
}

//======================================================================
