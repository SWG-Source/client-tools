//======================================================================
//
// SwgCuiAuctionSell.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgclientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAuctionSell.h"

#include "clientGame/AuctionManagerClient.h"
#include "clientGame/AuctionManagerClientData.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientUserInterface/CuiAuctionManager.h"
#include "clientUserInterface/CuiContainerSelectionChanged.h"
#include "clientUserInterface/CuiIconManagerCallback.h"
#include "clientUserInterface/CuiInventoryManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMenuInfoTypes.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringIdsAuction.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedGame/RadialMenuManager.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/CachedNetworkId.h"
#include "swgClientUserInterface/SwgCuiContainerProviderDefault.h"
#include "swgClientUserInterface/SwgCuiInventoryContainer.h"
#include "swgClientUserInterface/SwgCuiInventoryInfo.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "UIData.h"
#include "UIPage.h"
#include "UITabbedPane.h"
#include "UIText.h"
#include "UITextbox.h"
#include "UIUtils.h"

//----------------------------------------------------------------------

namespace SwgCuiAuctionSellNamespace
{
	//----------------------------------------------------------------------

	class IconCallback : public CuiIconManagerCallback
	{
	public:
		IconCallback  (SwgCuiAuctionSell & _sell, bool _isInventory);
		~IconCallback () {}
		virtual void overrideTooltip(const ClientObject & obj, Unicode::String & str) const  
		{
			NetworkId const & objId = obj.getNetworkId();

			bool const showTooltipInfo = ObjectAttributeManager::hasExtendedTooltipInformation(objId);

			if (showTooltipInfo) 
			{
				AuctionManagerClient::Data const * const data = AuctionManagerClient::findAuction(objId);
				if (data)
				{
					ObjectAttributeManager::formatAttributes(data->details.propertyList, str, NULL, NULL, false, true);
					
					if (!data->details.userDescription.empty()) 
					{
						str.append(1, '\n');
						str.append(data->details.userDescription);
					}
				}
				else
				{
					AuctionManagerClient::requestDetails(objId);
				}
			}
		}
		
		//		virtual void            overrideCursor      (const ClientObject & obj, UICursor *& cursor) const;
		virtual bool overrideDoubleClick (const UIWidget & viewer) const;

		SwgCuiAuctionSell * sell;

		bool                isInventory;
	};
	
	//----------------------------------------------------------------------

	IconCallback::IconCallback (SwgCuiAuctionSell & _sell, bool _isInventory) :
		CuiIconManagerCallback (),
		sell                   (&_sell),
		isInventory            (_isInventory)
	{
	}

	//----------------------------------------------------------------------
	
	bool IconCallback::overrideDoubleClick (const UIWidget & wid) const
	{
		NOT_NULL (sell);

		const CuiWidget3dObjectListViewer * const viewer = dynamic_cast<const CuiWidget3dObjectListViewer *>(&wid);

		if (viewer)
		{
			ClientObject * const obj = const_cast<ClientObject *>(dynamic_cast<const ClientObject *>(viewer->getLastObject ()));
			if (obj)
			{
				sell->onObjectIconDoubleClick (*obj, isInventory);
			}
		}

		return true;
	}
	
	//----------------------------------------------------------------------

	class ContainerFilter : public SwgCuiInventoryContainerFilter
	{
	public:
		~ContainerFilter ();
		ContainerFilter (SwgCuiAuctionSell & _sell, bool isInventory);
	};

	//----------------------------------------------------------------------
		
	ContainerFilter::~ContainerFilter () 
	{
		delete iconCallback;
		iconCallback = 0;
	}

	//----------------------------------------------------------------------

	ContainerFilter::ContainerFilter (SwgCuiAuctionSell & _sell, bool isInventory) :
	SwgCuiInventoryContainerFilter (new IconCallback (_sell, isInventory))
	{
	}

	//----------------------------------------------------------------------
}

using namespace SwgCuiAuctionSellNamespace;

//----------------------------------------------------------------------

SwgCuiAuctionSell::SwgCuiAuctionSell (UIPage & page) :
CuiMediator            ("SwgCuiAuctionSell", page),
UIEventCallback        (),
m_containerInventory   (0),
m_containerDatapad     (0),
m_info                 (0),
m_textDesc             (0),
m_checkInstantSale     (0),
m_checkAuction         (0),
m_textboxInstantSale   (0),
m_textboxMinBid        (0),
m_textboxDays          (0),
m_textboxHours         (0),
m_textboxMins          (0),
m_textboxVendorPrice   (0),
m_buttonOk             (0),
m_buttonCancel         (0),
m_pageRight            (0),
m_textDescStock        (0),
m_textObjectNameStock  (0),
m_viewerStock          (0),
m_checkPremium         (0),
m_callback             (new MessageDispatch::Callback),
m_containerProviderInventory    (new SwgCuiContainerProviderDefault),
m_containerProviderDatapad      (new SwgCuiContainerProviderDefault),
m_containerFilterInventory      (0),
m_containerFilterDatapad        (0),
m_tabs                          (0),
m_itemsToSellFromStock          ()
{
	setState    (MS_closeable);
	setState    (MS_closeDeactivates);

	{
		UIPage * inventoryPage = 0;
		getCodeDataObject (TUIPage, inventoryPage, "inventoryContainer");
		m_containerInventory      = new SwgCuiInventoryContainer (*inventoryPage);
		m_containerInventory->fetch ();

		m_containerFilterInventory = new ContainerFilter (*this, true);
		m_containerInventory->setContainerProvider (m_containerProviderInventory);
		m_containerInventory->setFilter            (m_containerFilterInventory);
	}

	{
		UIPage * inventoryPage = 0;
		getCodeDataObject (TUIPage, inventoryPage, "datapadContainer");
		m_containerDatapad      = new SwgCuiInventoryContainer (*inventoryPage);
		m_containerDatapad->fetch ();

		m_containerFilterDatapad = new ContainerFilter (*this, false);
		m_containerDatapad->setContainerProvider (m_containerProviderDatapad);
		m_containerDatapad->setFilter            (m_containerFilterDatapad);
	}	    
	
	{
		UIPage * infoPage = 0;
		getCodeDataObject (TUIPage, infoPage, "info");
		m_info      = new SwgCuiInventoryInfo (*infoPage);
		m_info->fetch ();
		m_info->connectToSelectionTransceiver (m_containerInventory->getTransceiverSelection ());
		m_info->connectToSelectionTransceiver (m_containerDatapad->getTransceiverSelection   ());
	}
	
	getCodeDataObject (TUIText,         m_textDesc,            "textDesc");
	getCodeDataObject (TUICheckbox,     m_checkInstantSale,    "checkInstantSale");
	getCodeDataObject (TUICheckbox,     m_checkAuction,        "checkAuction");
	getCodeDataObject (TUITextbox,      m_textboxInstantSale,  "textboxInstantSale");
	getCodeDataObject (TUITextbox,      m_textboxMinBid,       "textboxMinBid");
	getCodeDataObject (TUITextbox,      m_textboxDays,         "textboxDays");
	getCodeDataObject (TUITextbox,      m_textboxHours,        "textboxHours");
	getCodeDataObject (TUITextbox,      m_textboxMins,         "textboxMins");
	getCodeDataObject (TUITextbox,      m_textboxVendorPrice,  "textboxVendorPrice");	
	getCodeDataObject (TUIButton,       m_buttonOk,            "buttonOk");
	getCodeDataObject (TUIButton,       m_buttonCancel,        "buttonCancel");
	getCodeDataObject (TUIPage,         m_pageRight,           "right");
	getCodeDataObject (TUIText,         m_textDescStock,       "textDescStock");
	getCodeDataObject (TUIText,         m_textObjectNameStock, "textObjectNameStock");
	getCodeDataObject (TUITabbedPane,   m_tabs,                "tabs");

	{
		UIWidget * widget = 0;
		getCodeDataObject (TUIWidget, widget, "viewerStock");
		m_viewerStock = NON_NULL (dynamic_cast<CuiWidget3dObjectListViewer *>(widget)); 
		m_viewerStock->setCameraLodBias (2.0f);
		m_viewerStock->SetLocalTooltip (CuiStringIds::tooltip_viewer_3d_controls.localize ());
	}

	getCodeDataObject (TUICheckbox, m_checkPremium,      "checkPremium");

	m_textObjectNameStock->SetPreLocalized         (true);
	m_textDescStock->SetPreLocalized               (true);

	if (m_textboxVendorPrice)
		m_textboxVendorPrice->SetMaxIntegerLength(9);

	registerMediatorObject (*m_textDesc,           true);
	registerMediatorObject (*m_checkInstantSale,   true);
	registerMediatorObject (*m_checkAuction,       true);
	registerMediatorObject (*m_textboxInstantSale, true);
	registerMediatorObject (*m_textboxMinBid,      true);
	registerMediatorObject (*m_textboxDays,        true);
	registerMediatorObject (*m_textboxHours,       true);
	registerMediatorObject (*m_textboxMins,        true);
	registerMediatorObject (*m_textboxVendorPrice, true);
	registerMediatorObject (*m_buttonOk,           true);
	registerMediatorObject (*m_buttonCancel,       true);
	registerMediatorObject (*m_tabs,               true);

	m_checkInstantSale->SetChecked (true);
	m_checkPremium->SetChecked (false);
}

//----------------------------------------------------------------------

SwgCuiAuctionSell::~SwgCuiAuctionSell ()
{
	m_info->disconnectFromSelectionTransceiver (m_containerInventory->getTransceiverSelection ());
	m_info->disconnectFromSelectionTransceiver (m_containerDatapad->getTransceiverSelection ());

	m_containerInventory->setFilter (0);
	m_containerInventory->setContainerProvider (0);
	m_containerInventory->release ();
	m_containerInventory          = 0;

	delete m_containerFilterInventory;
	m_containerFilterInventory = 0;

	m_containerDatapad->setFilter (0);
	m_containerDatapad->setContainerProvider (0);
	m_containerDatapad->release ();
	m_containerDatapad          = 0;

	delete m_containerFilterDatapad;
	m_containerFilterDatapad = 0;

	m_info->release ();
	m_info               = 0;

	m_textDesc           = 0;
	m_checkInstantSale   = 0;
	m_checkAuction       = 0;
	m_textboxInstantSale = 0;
	m_textboxMinBid      = 0;
	m_textboxDays        = 0;
	m_textboxHours       = 0;
	m_textboxMins        = 0;
	m_textboxVendorPrice = 0;
	m_buttonOk           = 0;
	m_buttonCancel       = 0;

	delete m_containerProviderInventory;
	m_containerProviderInventory = 0;

	delete m_containerProviderDatapad;
	m_containerProviderDatapad = 0;

	delete m_callback;
	m_callback = 0;
}

//----------------------------------------------------------------------

void SwgCuiAuctionSell::performActivate   ()
{
	float range = 0.0f;
	if (RadialMenuManager::getRangeForMenuType (Cui::MenuInfoTypes::TERMINAL_AUCTION_USE, range))
	{
		setAssociatedObjectId (AuctionManagerClient::getMarketObjectId ());
		setMaxRangeFromObject (range);
	}

	CuiManager::requestPointer (true);

	if (AuctionManagerClient::isAtCommodityMarket ())
		m_textboxVendorPrice->SetVisible (false);
	else
		m_textboxVendorPrice->SetVisible (true);

	m_itemsToSellFromStock = AuctionManagerClient::getItemsToSellFromStock ();
	const NetworkId itemToSell = (m_itemsToSellFromStock.empty() ? NetworkId::cms_invalid : m_itemsToSellFromStock.front());

	if (!itemToSell.isValid ())
	{
		m_tabs->SetVisible (true);

		m_info->activate ();

		Unicode::String sevenStr;
		UIUtils::FormatInteger (sevenStr, 7);
		
		Unicode::String zeroStr;
		UIUtils::FormatInteger (zeroStr, 0);
		
		m_textboxDays->SetLocalText        (sevenStr);
		m_textboxHours->SetLocalText       (zeroStr);
		m_textboxMins->SetLocalText        (zeroStr);
		m_textboxVendorPrice->SetLocalText (zeroStr);

		if (!m_containerInventory->getContainerObject ())
		{
			ClientObject * const inv = CuiInventoryManager::getPlayerInventory ();
			m_containerInventory->setContainerObject (inv, std::string ());
		}

		if (!m_containerDatapad->getContainerObject ())
		{
			ClientObject * const datapad = CuiInventoryManager::getPlayerDatapad();
			m_containerDatapad->setContainerObject (datapad, std::string ());
		}
		
		bool canMakePremium = AuctionManagerClient::isAtCommodityMarket ();
		
		if (canMakePremium)
		{
			const CreatureObject * const player = Game::getPlayerCreature ();
			if (player)
			{
				static const std::string cmdname_premium_ok = "premium_auctions";
				const std::map<std::string, int> & sv = player->getCommands ();
				canMakePremium = (sv.find(cmdname_premium_ok) != sv.end ());
			}
		}
		
		m_checkPremium->SetVisible (canMakePremium);
		
		m_callback->connect (m_containerInventory->getTransceiverSelection (), *this, &SwgCuiAuctionSell::onSelectionChanged);
		m_callback->connect (m_containerDatapad->getTransceiverSelection (),   *this, &SwgCuiAuctionSell::onSelectionChanged);

		OnTabbedPaneChanged  (m_tabs);
//		onSelectionChanged (CuiContainerSelectionChanged::Payload (0, 0));

		m_textDesc->Clear ();
	}
	else
	{
		m_tabs->SetVisible (false);

		m_pageRight->SetEnabled (true);

		m_checkPremium->SetVisible (false);

		m_callback->connect (*this, &SwgCuiAuctionSell::onDetailsReceived,       static_cast<AuctionManagerClient::Messages::DetailsReceived*> (0));

		const Data * const data = AuctionManagerClient::findAuction (itemToSell);
		if (data)
		{
			m_textDesc->Clear ();
			populateDetails (*data);
		}
		else
		{
			WARNING (true, ("Unable to find item to sell from stock"));
			deactivate ();
			return;
		}
	}

	m_callback->connect (*this, &SwgCuiAuctionSell::onCreateAuctionResponse, static_cast<AuctionManagerClient::Messages::CreateAuctionResponse *>(0));
}

//----------------------------------------------------------------------

void SwgCuiAuctionSell::OnTabbedPaneChanged (UIWidget * context)
{
	if (context == m_tabs)
	{
		if (m_tabs->GetActiveTab () == 0)
		{			
			m_containerDatapad->deactivate ();
			
			m_containerInventory->activate ();
			m_containerInventory->setViewType (m_containerDatapad->getViewType ());
		}
		else
		{			
			m_containerInventory->deactivate ();
			
			m_containerDatapad->activate ();
			m_containerDatapad->setViewType (m_containerInventory->getViewType ());
		}

		onSelectionChanged (CuiContainerSelectionChanged::Payload (0, 0));
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionSell::performDeactivate ()
{
	CuiManager::requestPointer (false);

	m_callback->disconnect (*this, &SwgCuiAuctionSell::onCreateAuctionResponse, static_cast<AuctionManagerClient::Messages::CreateAuctionResponse *>(0));
	m_callback->disconnect (*this, &SwgCuiAuctionSell::onDetailsReceived,       static_cast<AuctionManagerClient::Messages::DetailsReceived*> (0));

	m_callback->disconnect (m_containerInventory->getTransceiverSelection (), *this, &SwgCuiAuctionSell::onSelectionChanged);
	m_callback->disconnect (m_containerDatapad->getTransceiverSelection (),   *this, &SwgCuiAuctionSell::onSelectionChanged);

	if (m_containerInventory->isActive ())
		m_containerDatapad->setViewType (m_containerInventory->getViewType ());
	else if (m_containerDatapad->isActive ())
		m_containerInventory->setViewType (m_containerDatapad->getViewType ());

	m_containerInventory->deactivate ();
	m_containerDatapad->deactivate ();

	m_info->deactivate ();

	setIsUpdating (false);
}

//----------------------------------------------------------------------

void SwgCuiAuctionSell::OnButtonPressed (UIWidget * context)
{
	if (context == m_buttonCancel)
		closeThroughWorkspace ();
	else if (context == m_buttonOk)
	{
		ok ();
	}
}


//----------------------------------------------------------------------

void SwgCuiAuctionSell::OnTextboxChanged (UIWidget * context)
{
	if (context == m_textboxDays)
	{
		int val = m_textboxDays->GetNumericIntegerValue ();
		if (val > 7)
			m_textboxDays->SetLocalText (Unicode::narrowToWide ("7"));
	}
	else if (context == m_textboxHours)
	{
		int val = m_textboxHours->GetNumericIntegerValue ();
		if (val > 24)
			m_textboxHours->SetLocalText (Unicode::narrowToWide ("24"));
	}
	else if (context == m_textboxMins)
	{
		int val = m_textboxMins->GetNumericIntegerValue ();
		if (val > 60)
			m_textboxMins->SetLocalText (Unicode::narrowToWide ("60"));
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionSell::handleVendorOk ()
{
	bool isOwner = false;
	
	if (!AuctionManagerClient::isAtVendor (isOwner))
	{
		deactivate ();
		return;
	}

	const int price = m_textboxVendorPrice->GetNumericIntegerValue ();
	
	if (price <= 0)
	{
		CuiMessageBox::createInfoBox (CuiStringIdsAuction::err_vendor_sell_invalid_price.localize ());
		return;
	}

	const Unicode::String & desc = m_textDesc->GetLocalText ();

	NetworkId itemId;

	bool isVendorStockroomTransfer = false;

	if (m_containerInventory->isActive () || m_containerDatapad->isActive ())
	{
		ClientObject * const obj = m_containerInventory->isActive () ? m_containerInventory->getLastSelection () : m_containerDatapad->getLastSelection ();
		if (obj)
		{
			itemId = obj->getNetworkId ();
		}	
	}
	else
	{
		itemId = (m_itemsToSellFromStock.empty() ? NetworkId::cms_invalid : m_itemsToSellFromStock.front());
		isVendorStockroomTransfer = true;
	}

	if (itemId.isValid ())
	{
		CuiAuctionManager::handleVendorSale (itemId, price, desc, true, false, isVendorStockroomTransfer);
	}
	else
	{
		CuiMessageBox::createInfoBox (CuiStringIdsAuction::err_vendor_sell_no_item_selected.localize ());
		return;
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionSell::handleAuctionOk ()
{
	const bool auction = m_checkAuction->IsChecked ();

	int price = 0;
	int totalSeconds = 60 * 60 * 24 * 7; // one week

	//@todo: update this from a checkbox
	bool isPremium = m_checkPremium->IsVisible () && m_checkPremium->IsChecked ();

	if (auction)
	{
		Unicode::String priceVal = m_textboxMinBid->GetLocalText();
		price = atoi(Unicode::wideToNarrow(priceVal).c_str());

		if (price <= 0)
		{
			CuiMessageBox::createInfoBox (CuiStringIdsAuction::err_sell_invalid_min_bid.localize ());
			return;
		}

		const int days   = m_textboxDays->GetNumericIntegerValue  ();
		const int hours  = m_textboxHours->GetNumericIntegerValue ();
		const int mins   = m_textboxMins->GetNumericIntegerValue  ();
		
		totalSeconds = mins * 60 + (hours * 60 * 60) + (days * 60 * 60 * 24);
		
		if (totalSeconds <= 0)
		{
			CuiMessageBox::createInfoBox (CuiStringIdsAuction::err_sell_invalid_time.localize ());
			return;
		}
	}
	else
	{
		Unicode::String priceVal = m_textboxInstantSale->GetLocalText();
		price = atoi(Unicode::wideToNarrow(priceVal).c_str());
		if (price <= 0)
		{
			CuiMessageBox::createInfoBox (CuiStringIdsAuction::err_sell_invalid_instant_price.localize ());
			return;
		}
	}
	
	const Unicode::String & desc = m_textDesc->GetLocalText ();
	
	ClientObject * const obj = m_containerInventory->isActive () ? m_containerInventory->getLastSelection () : m_containerDatapad->getLastSelection ();
	if (obj)
	{
		const NetworkId & itemId = obj->getNetworkId ();
		CuiAuctionManager::handleAuctionSale (itemId, price, totalSeconds, !auction, desc, true, isPremium);
	}
	else
	{
		CuiMessageBox::createInfoBox (CuiStringIdsAuction::err_sell_no_item_selected.localize ());
		return;
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionSell::ok ()
{
	//-- vendor
	if (m_textboxVendorPrice->IsVisible ())
		handleVendorOk ();
	else
		handleAuctionOk ();
}

//----------------------------------------------------------------------

void SwgCuiAuctionSell::onCreateAuctionResponse (const AuctionManagerClient::Messages::CreateAuctionResponse::Payload & payload)
{
	// if there are multiple items to sell, move on to the next one
	if (!m_itemsToSellFromStock.empty() && (payload.first.isValid()) && (m_itemsToSellFromStock.front() == payload.first))
	{
		// remove the item that just got put up for sale
		m_itemsToSellFromStock.erase(m_itemsToSellFromStock.begin());

		// any more item to sell?
		while (!m_itemsToSellFromStock.empty())
		{
			const Data * const data = AuctionManagerClient::findAuction(m_itemsToSellFromStock.front());
			if (data)
			{
				// handle sale of next item
				m_textDesc->Clear();
				populateDetails(*data);
				break;
			}
			else
			{
				// next item is invalid, skip it
				m_itemsToSellFromStock.erase(m_itemsToSellFromStock.begin());
			}
		}

		// no more items to sell, close Sell window
		if (m_itemsToSellFromStock.empty())
			deactivate();
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionSell::onObjectIconDoubleClick (ClientObject & obj, bool isInventory)
{
	if (obj.getVolumeContainerProperty ())
	{
		const int got = obj.getGameObjectType ();
		
		if (got == SharedObjectTemplate::GOT_misc_container || got == SharedObjectTemplate::GOT_misc_container_wearable)
		{
			if (isInventory)
				m_containerInventory->setContainerObject (&obj, std::string ());
			else
				m_containerDatapad->setContainerObject (&obj, std::string ());
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionSell::onSelectionChanged (const CuiContainerSelectionChanged::Payload &)
{
	if ((m_containerInventory->isActive () && m_containerInventory->getLastSelection ()) ||
		(m_containerDatapad->isActive () && m_containerDatapad->getLastSelection ()))
	{
		m_pageRight->SetEnabled (true);

		UIWidget * const focused = m_pageRight->GetFocusedLeafWidget ();

		if (!focused || !focused->IsEnabled ())
		{
			if (m_checkAuction->IsChecked ())
				m_textboxMinBid->SetFocus ();
			else
				m_textboxInstantSale->SetFocus ();
		}
	}
	else
	{
		m_pageRight->SetEnabled (false);
	}

	m_textDesc->Clear ();
}

//----------------------------------------------------------------------

void SwgCuiAuctionSell::onDetailsReceived       (const Data & data)
{
	const NetworkId itemToSell = (m_itemsToSellFromStock.empty() ? NetworkId::cms_invalid : m_itemsToSellFromStock.front());
	if (itemToSell.isValid ())
	{
		if (data.header.itemId == itemToSell)
		{
			//@todo: play 'data received' sound
			populateDetails (data);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionSell::populateDetails (const Data & data)
{
	m_viewerStock->clearObjects ();
	
	if (data.hasDetails)
	{
		Unicode::String str;
		ObjectAttributeManager::formatAttributes(data.details.propertyList, str, NULL, NULL, false);
		m_textDescStock->SetLocalText(str);

		if (!data.details.userDescription.empty()) 
			m_textDesc->SetLocalText(data.details.userDescription);
		else
			m_textDesc->Clear();

		ClientObject * const obj = AuctionManagerClient::getClientObjectForAuction (data.header.itemId);
		if (obj)
		{
			m_viewerStock->addObject (*obj);
			m_viewerStock->setCameraForceTarget  (true);
			m_viewerStock->setAutoZoomOutOnly    (false);
			m_viewerStock->recomputeZoom         ();
			m_viewerStock->setCameraForceTarget  (false);
			m_textObjectNameStock->SetLocalText  (obj->getLocalizedName ());
		}
		else
			m_textObjectNameStock->Clear         ();
	}
	else
	{
		m_textDescStock->SetLocalText (CuiStringIdsAuction::waiting_details.localize ());
		AuctionManagerClient::requestDetails (data.header.itemId);
		m_textObjectNameStock->Clear ();
		m_textDesc->Clear ();
	}

	if ((data.header.buyNowPrice > 0) && (m_textboxVendorPrice->IsVisible()))
	{
		Unicode::String buyNowPriceStr;
		UIUtils::FormatInteger(buyNowPriceStr, data.header.buyNowPrice);
		m_textboxVendorPrice->SetLocalText(buyNowPriceStr);
	}
}

//======================================================================
