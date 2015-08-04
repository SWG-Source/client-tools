//======================================================================
//
// SwgCuiAuctionList.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgclientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAuctionList.h"

#include "swgClientUserInterface/SwgCuiAuctionPaneTypes.h"
#include "clientUserInterface/CuiStringIdsAuction.h"
#include "clientGame/AuctionManagerClient.h"
#include "clientGame/Game.h"
#include "sharedObject/CachedNetworkId.h"

//======================================================================

SwgCuiAuctionList::SwgCuiAuctionList (UIPage & page) :
SwgCuiAuctionListBase ("SwgCuiAuctionList", page),
m_marketObjectId      ()
{
	setState (MS_closeable);
	setState (MS_closeDeactivates);
}

//----------------------------------------------------------------------

void SwgCuiAuctionList::performActivate ()
{
	AuctionManagerClient::setIsAtCommodityMarket (true);

	//----------------------------------------------------------------------
	//-- setup the tabs

	static const int s_numTabs = 5;
	static const TabInfo s_tabInfo [s_numTabs] =
	{
		TabInfo ("SwgCuiAuctionListPaneAll",       SwgCuiAuctionPaneTypes::T_all,       "pageAll",          CuiStringIdsAuction::list_tab_all),
		TabInfo ("SwgCuiAuctionListPaneMyBids",    SwgCuiAuctionPaneTypes::T_myBids,    "pageMyBids",       CuiStringIdsAuction::list_tab_bids),
		TabInfo ("SwgCuiAuctionListPaneMySales",   SwgCuiAuctionPaneTypes::T_mySales,   "pageMySales",      CuiStringIdsAuction::list_tab_sales),
		TabInfo ("SwgCuiAuctionListPaneAvailable", SwgCuiAuctionPaneTypes::T_available, "pageAvailable",    CuiStringIdsAuction::list_tab_available),
		TabInfo ("SwgCuiAuctionListPaneLocation",  SwgCuiAuctionPaneTypes::T_location,  "pageVendorSLocation",     CuiStringIdsAuction::list_tab_location, true)
	};

	const CachedNetworkId & marketObjectId = AuctionManagerClient::getMarketObjectId ();

	if ((marketObjectId != m_marketObjectId) || Game::getSinglePlayer())
		setTabs (s_tabInfo, s_numTabs);

	m_marketObjectId = marketObjectId;

	SwgCuiAuctionListBase::performActivate ();
}

//----------------------------------------------------------------------

void SwgCuiAuctionList::performDeactivate ()
{
	SwgCuiAuctionListBase::performDeactivate ();
}

//======================================================================
