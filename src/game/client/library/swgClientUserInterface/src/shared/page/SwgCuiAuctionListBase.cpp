//======================================================================
//
// SwgCuiAuctionListBase.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgclientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAuctionListBase.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIMessage.h"
#include "UITabbedPane.h"
#include "UITable.h"
#include "UIText.h"
#include "UnicodeUtils.h"
#include "clientGame/AuctionManagerClient.h"
#include "clientGame/AuctionManagerClientData.h"
#include "clientGame/ClientObject.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiAuctionManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMenuInfoTypes.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiMoney.h"
#include "clientUserInterface/CuiStringIdsAuction.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedGame/RadialMenuManager.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/CachedNetworkId.h"
#include "swgClientUserInterface/SwgCuiAuctionFilter.h"
#include "swgClientUserInterface/SwgCuiAuctionListPane.h"
#include "swgClientUserInterface/SwgCuiAuctionPaneTypes.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include <algorithm>
#include <list>
#include <map>

//----------------------------------------------------------------------

SwgCuiAuctionListBase::SwgCuiAuctionListBase (const char * const name, UIPage & page) :
CuiMediator        (name, page),
UIEventCallback    (),
m_callback         (new MessageDispatch::Callback),
m_tabs             (0),
m_buttonExit       (0),
m_buttonDetails    (0),
m_buttonRefresh    (0),
m_caption          (0),
m_paneMap          (new PaneMap),
m_type             (SwgCuiAuctionPaneTypes::T_all),
m_tabTypes         (new IntVector),
m_tabInfoVector    (new TabInfoVector),
m_money            (0),
m_elapsedTime      (0.0f)
{
	setState    (MS_closeable);
	setState    (MS_closeDeactivates);

	Zero (m_textStatus);

	getCodeDataObject (TUITabbedPane,     m_tabs,              "tabs");
	getCodeDataObject (TUIButton,         m_buttonExit,        "buttonExit");
	getCodeDataObject (TUIButton,         m_buttonDetails,     "buttonDetails");
	getCodeDataObject (TUIButton,         m_buttonRefresh,     "buttonRefresh");
	getCodeDataObject (TUIText,           m_caption,           "caption");
	getCodeDataObject (TUIText,           m_textStatus [0],       "textStatus0");
	getCodeDataObject (TUIText,           m_textStatus [1],       "textStatus1");

	{
		UIPage * pageMoney = 0;
		getCodeDataObject (TUIPage,           pageMoney,           "money");
		m_money = new CuiMoney (*pageMoney);
		m_money->fetch ();
	}

	m_caption->SetPreLocalized (true);
	m_textStatus [0]->SetPreLocalized (true);
	m_textStatus [1]->SetPreLocalized (true);

	m_textStatus [0]->Clear ();
	m_textStatus [1]->Clear ();

	registerMediatorObject (*m_tabs,            true);
	registerMediatorObject (*m_buttonExit,      true);
	registerMediatorObject (*m_buttonDetails,   true);
	registerMediatorObject (*m_buttonRefresh,   true);

	m_tabs->SetActiveTab (-1);
}

//----------------------------------------------------------------------

SwgCuiAuctionListBase::~SwgCuiAuctionListBase ()
{
	m_money->release ();
	m_money = 0;

	delete m_tabInfoVector;
	m_tabInfoVector = 0;

	delete m_callback;
	m_callback = 0;

	clearPanes ();

	delete m_paneMap;
	m_paneMap = 0;

	delete m_tabTypes;
	m_tabTypes = 0;
}

//----------------------------------------------------------------------

void SwgCuiAuctionListBase::performActivate ()
{
	m_textStatus [0]->Clear ();
	m_textStatus [1]->Clear ();

	m_callback->connect (*this, &SwgCuiAuctionListBase::onRetrieveRequested, static_cast<AuctionManagerClient::Messages::RetrieveRequested*> (0));
	m_callback->connect (*this, &SwgCuiAuctionListBase::onItemRetrieved,     static_cast<CuiAuctionManager::Messages::Retrieved*> (0));
	m_callback->connect (*this, &SwgCuiAuctionListBase::onWithdrawRequested, static_cast<AuctionManagerClient::Messages::WithdrawRequested*> (0));
	m_callback->connect (*this, &SwgCuiAuctionListBase::onWithdrawn,         static_cast<CuiAuctionManager::Messages::Withdrawn*> (0));

	float range = 0.0f;
	if (RadialMenuManager::getRangeForMenuType (Cui::MenuInfoTypes::TERMINAL_AUCTION_USE, range))
	{
		setAssociatedObjectId (AuctionManagerClient::getMarketObjectId ());
		setMaxRangeFromObject (range);
	}

	if (m_tabs->GetTabCount () <= 0)
		setupTabs ();

	getPage ().SetEnabled (true);

	CuiManager::requestPointer (true);
	
	setActiveView (m_type);
	
	setupCurrentTab ();

	requestUpdate (true);

	AuctionManagerClient::requestItemTypeList();
	AuctionManagerClient::requestResourceTypeList();

	m_money->activate ();

	const Object * const player = Game::getPlayer ();
	if (player)
		m_money->setId (player->getNetworkId ());

	setIsUpdating (true);

	updateStatusText ();
}

//----------------------------------------------------------------------

void SwgCuiAuctionListBase::performDeactivate ()
{
	m_callback->disconnect (*this, &SwgCuiAuctionListBase::onRetrieveRequested, static_cast<AuctionManagerClient::Messages::RetrieveRequested*> (0));
	m_callback->disconnect (*this, &SwgCuiAuctionListBase::onItemRetrieved,     static_cast<CuiAuctionManager::Messages::Retrieved*> (0));
	m_callback->disconnect (*this, &SwgCuiAuctionListBase::onWithdrawRequested, static_cast<AuctionManagerClient::Messages::WithdrawRequested*> (0));
	m_callback->disconnect (*this, &SwgCuiAuctionListBase::onWithdrawn,         static_cast<CuiAuctionManager::Messages::Withdrawn*> (0));

	CuiManager::requestPointer (false);

	setActiveView (m_type, true);

	m_money->deactivate ();

	setIsUpdating (false);
}

//----------------------------------------------------------------------

bool SwgCuiAuctionListBase::close                     ()
{
	CuiMediator::close ();

	deactivate ();

	clearPanes ();
	m_tabs->Clear ();
	m_tabTypes->clear ();

	AuctionManagerClient::reset ();
	CuiMediatorFactory::deactivateInWorkspace (CuiMediatorTypes::WS_AuctionBid);
	CuiMediatorFactory::deactivateInWorkspace (CuiMediatorTypes::WS_AuctionBuy);
	CuiMediatorFactory::deactivateInWorkspace (CuiMediatorTypes::WS_AuctionSell);
	CuiMediatorFactory::deactivateInWorkspace (CuiMediatorTypes::WS_AuctionDetails);
	CuiMediatorFactory::deactivateInWorkspace (CuiMediatorTypes::WS_VendorSetPrice);
	return true;
}

//----------------------------------------------------------------------

void SwgCuiAuctionListBase::setTabs (const TabInfo * tabInfos, int numTabs)
{
	m_tabInfoVector->clear ();
	if (tabInfos)
	{
		m_tabInfoVector->assign (tabInfos, tabInfos + numTabs);
		m_type = tabInfos [0].type;
	}
	else
		m_type = SwgCuiAuctionPaneTypes::T_all;

	Unicode::String cpt;

	std::string planet;
	std::string region;
	std::string name;

	AuctionManagerClient::getMarketLocationStrings (planet, region, name);
	
	switch (m_type)
	{
	case SwgCuiAuctionPaneTypes::T_all:
	case SwgCuiAuctionPaneTypes::T_mySales:
	case SwgCuiAuctionPaneTypes::T_myBids:
	case SwgCuiAuctionPaneTypes::T_available:
		{
			cpt = CuiStringIdsAuction::cpt_auction_list.localize ();
			cpt += StringId::decodeString (Unicode::narrowToWide (region));
		}
		break;
	case SwgCuiAuctionPaneTypes::T_vendorSellerSelling:
	case SwgCuiAuctionPaneTypes::T_vendorSellerOffers:
	case SwgCuiAuctionPaneTypes::T_vendorSellerStockroom:
		{
			cpt = CuiStringIdsAuction::cpt_vendor_owner_list.localize ();
			cpt += StringId::decodeString (Unicode::narrowToWide (name));
		}
		break;
	case SwgCuiAuctionPaneTypes::T_vendorBuyerSelling:
	case SwgCuiAuctionPaneTypes::T_vendorBuyerOffers:	
		{
			cpt = CuiStringIdsAuction::cpt_vendor_list.localize ();
			cpt += StringId::decodeString (Unicode::narrowToWide (name));
		}
		break;
	}

	m_caption->SetLocalText (cpt);
	setupTabs ();
}

//----------------------------------------------------------------------

void SwgCuiAuctionListBase::clearPanes ()
{
	for (PaneMap::iterator it = m_paneMap->begin (); it != m_paneMap->end (); ++it)
	{
		SwgCuiAuctionListPane * const pane = (*it).second;
		pane->release ();
	}
	m_paneMap->clear ();
}

//----------------------------------------------------------------------

void SwgCuiAuctionListBase::setupTabs ()
{
	//----------------------------------------------------------------------
	//-- setup the tabs

	clearPanes ();
	m_tabs->Clear ();
	m_tabTypes->clear ();

	for (TabInfoVector::const_iterator it = m_tabInfoVector->begin (); it < m_tabInfoVector->end (); ++it)
	{
		const TabInfo & tabInfo = *it;

		UIPage * page = 0;
		getCodeDataObject (TUIPage,       page,          tabInfo.page);
		NOT_NULL (page);
		SwgCuiAuctionListPane * const pane = new SwgCuiAuctionListPane (tabInfo.name, *page, tabInfo.type, *this, tabInfo.disableBuyInDetails);
		pane->fetch ();
		(*m_paneMap) [tabInfo.type] = pane;
		m_tabTypes->push_back (tabInfo.type);
		UIData * const data = m_tabs->AppendTab (tabInfo.label->localize (), page);
		if (data)
			data->SetName (tabInfo.name);
	}

	m_tabs->Link ();
}

//----------------------------------------------------------------------

void SwgCuiAuctionListBase::OnTabbedPaneChanged (UIWidget * context)
{
	if (context == m_tabs)
	{
		setupCurrentTab ();
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionListBase::OnButtonPressed (UIWidget * context)
{
	if (context == m_buttonExit)
	{
		closeThroughWorkspace ();
	}
	else if (context == m_buttonDetails)
		showDetails ();
	else if (context == m_buttonRefresh)
		requestUpdate (false);
}

//----------------------------------------------------------------------

void SwgCuiAuctionListBase::setActiveView (int type, bool forceDeactivate)
{
	m_type = type;

	for (PaneMap::iterator it = m_paneMap->begin (); it != m_paneMap->end (); ++it)
	{
		const int baseType                 = (*it).first;
		SwgCuiAuctionListPane * const pane = (*it).second;
		NOT_NULL (pane);

		if (forceDeactivate)
			pane->deactivate ();
		else
		{
			if (baseType == type)
			{
				pane->activate ();
			}
			else
				pane->deactivate ();
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionListBase::setupCurrentTab ()
{
	const long activeTab = m_tabs->GetActiveTab ();

	if (activeTab < 0)
		setActiveView (SwgCuiAuctionPaneTypes::T_all, true);
	else
	{
		DEBUG_FATAL (activeTab >= static_cast<int>(m_tabTypes->size ()), ("bad"));
		setActiveView ((*m_tabTypes) [static_cast<size_t>(activeTab)]);
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionListBase::requestUpdate (bool optional)
{
	SwgCuiAuctionListPane * const pane = NON_NULL ((*m_paneMap) [m_type]);
	pane->requestUpdate (optional, 0);
}

//----------------------------------------------------------------------

void SwgCuiAuctionListBase::showDetails () const
{
	SwgCuiAuctionListPane * const pane = NON_NULL ((*m_paneMap) [m_type]);
	pane->showDetails (pane->findSelectedAuction (true));
}

//----------------------------------------------------------------------

void SwgCuiAuctionListBase::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	m_elapsedTime += deltaTimeSecs;

	if (m_elapsedTime > 0.25f)	
	{
		m_elapsedTime = 0.0f;

		AuctionManagerClient::AuctionListType alt = AuctionManagerClient::T_numTypes;
		switch (m_type)
		{
		case SwgCuiAuctionPaneTypes::T_all:
			alt = AuctionManagerClient::T_all;
			break;
		case SwgCuiAuctionPaneTypes::T_mySales:
			alt = AuctionManagerClient::T_mySales;
			break;
		case SwgCuiAuctionPaneTypes::T_myBids:
			alt = AuctionManagerClient::T_myBids;
			break;
		case SwgCuiAuctionPaneTypes::T_available:
			alt = AuctionManagerClient::T_available;
			break;
		case SwgCuiAuctionPaneTypes::T_vendorSellerSelling:
		case SwgCuiAuctionPaneTypes::T_vendorBuyerSelling:
			alt = AuctionManagerClient::T_vendorSelling;
			break;
		case SwgCuiAuctionPaneTypes::T_vendorSellerOffers:
		case SwgCuiAuctionPaneTypes::T_vendorBuyerOffers:
			alt = AuctionManagerClient::T_vendorOffers;
			break;
		case SwgCuiAuctionPaneTypes::T_vendorSellerStockroom:
			alt = AuctionManagerClient::T_vendorStockroom;
			break;
		case SwgCuiAuctionPaneTypes::T_location:
			alt = AuctionManagerClient::T_location;
			break;
		
		}

		m_buttonRefresh->SetVisible (!AuctionManagerClient::isListRequestOutstanding (alt));
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionListBase::onRetrieveRequested       (const AuctionManagerClient::Messages::RetrieveRequested::Payload &)
{
	updateStatusText ();
}

//----------------------------------------------------------------------

void SwgCuiAuctionListBase::onItemRetrieved           (const CuiAuctionManager::Messages::Retrieved::Payload &)
{
	updateStatusText ();
}

//----------------------------------------------------------------------

void SwgCuiAuctionListBase::onWithdrawRequested       (const AuctionManagerClient::Messages::RetrieveRequested::Payload &)
{
	updateStatusText ();
}

//----------------------------------------------------------------------

void SwgCuiAuctionListBase::onWithdrawn           (const CuiAuctionManager::Messages::Withdrawn::Payload &)
{
	updateStatusText ();
}

//----------------------------------------------------------------------

void SwgCuiAuctionListBase::updateStatusText ()
{
	int statusIndex = 0;
	char buf [128];
	const size_t buf_size = sizeof (buf);

	{
		const int count = CuiAuctionManager::getOutstandingRetrieveRequestCount ();
		
		if (count)
		{
			if (count == 1)
				snprintf (buf, buf_size, "Retrieving %d item.", count);
			else
				snprintf (buf, buf_size, "Retrieving %d items.", count);
			
			static const Unicode::String retrieve_prefix = Unicode::narrowToWide ("\\#pcontrast1 ");

			Unicode::String str = retrieve_prefix + Unicode::narrowToWide (buf);
			m_textStatus [statusIndex]->SetLocalText (str);
			++statusIndex;
		}
	}

	if (statusIndex >= NumTextStatus)
		return;

	{
		const int count = CuiAuctionManager::getOutstandingWithdrawRequestCount ();
		
		if (count)
		{
			if (count == 1)
				snprintf (buf, buf_size, "Withdrawing %d item.", count);
			else
				snprintf (buf, buf_size, "Withdrawing %d items.", count);
			
			static const Unicode::String withdraw_prefix = Unicode::narrowToWide ("\\#pcontrast1 ");

			Unicode::String str = withdraw_prefix + Unicode::narrowToWide (buf);
			m_textStatus [statusIndex]->SetLocalText (str);
			++statusIndex;
		}
	}

	if (statusIndex >= NumTextStatus)
		return;

	for (int i = statusIndex; i < NumTextStatus; ++i)
		m_textStatus [i]->Clear ();
}

//======================================================================
