//======================================================================
//
// SwgCuiVendor.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgclientUserInterface.h"
#include "swgClientUserInterface/SwgCuiVendor.h"

#include "swgClientUserInterface/SwgCuiAuctionPaneTypes.h"
#include "clientUserInterface/CuiStringIdsAuction.h"
#include "clientGame/AuctionManagerClient.h"
#include "sharedObject/CachedNetworkId.h"

//======================================================================

namespace SwgCuiVendorNamespace
{

}

using namespace SwgCuiVendorNamespace;

//----------------------------------------------------------------------

SwgCuiVendor::SwgCuiVendor (UIPage & page) :
SwgCuiAuctionListBase ("SwgCuiVendor", page),
m_hasActivated        (false),
m_isOwner             (false),
m_vendorId            ()
{
	setState (MS_closeable);
	setState (MS_closeDeactivates);
}

//----------------------------------------------------------------------

void SwgCuiVendor::performActivate ()
{

	//----------------------------------------------------------------------
	//-- setup the tabs

	static const int s_numTabs = 6;
	static const TabInfo s_tabInfo [s_numTabs] =
	{
		TabInfo ("SwgCuiVendorPaneSellerSelling",   SwgCuiAuctionPaneTypes::T_vendorSellerSelling,     "pageVendorSSelling",      CuiStringIdsAuction::list_tab_vendor_seller_selling),
		TabInfo ("SwgCuiVendorPaneSellerOffers",    SwgCuiAuctionPaneTypes::T_vendorSellerOffers,      "pageVendorSOffers",       CuiStringIdsAuction::list_tab_vendor_seller_offers),
		TabInfo ("SwgCuiVendorPaneSellerStockroom", SwgCuiAuctionPaneTypes::T_vendorSellerStockroom,   "pageVendorSStock",        CuiStringIdsAuction::list_tab_vendor_seller_stockroom),
		TabInfo ("SwgCuiVendorPaneBuyerSelling",    SwgCuiAuctionPaneTypes::T_vendorBuyerSelling,      "pageVendorBSelling",      CuiStringIdsAuction::list_tab_vendor_buyer_selling),
		TabInfo ("SwgCuiVendorPaneBuyerOffers",     SwgCuiAuctionPaneTypes::T_vendorBuyerOffers,       "pageVendorBOffers",       CuiStringIdsAuction::list_tab_vendor_buyer_offers),
		TabInfo ("SwgCuiVendorPaneAvailable",       SwgCuiAuctionPaneTypes::T_available,               "pageAvailable",           CuiStringIdsAuction::list_tab_available)
	};

	bool isOwner = false;
	AuctionManagerClient::isAtVendor (isOwner);
	const CachedNetworkId & marketObjectId = AuctionManagerClient::getMarketObjectId ();

	if (isOwner)
	{
		if (!m_hasActivated || !m_isOwner || marketObjectId != m_vendorId)
			setTabs (s_tabInfo, 3);
	}
	else
	{
		if (!m_hasActivated || m_isOwner || marketObjectId != m_vendorId)
			setTabs (s_tabInfo + 3, 3);
	}

	m_vendorId = marketObjectId;
	m_hasActivated = true;
	m_isOwner      = isOwner;
	SwgCuiAuctionListBase::performActivate ();
}

//----------------------------------------------------------------------

void SwgCuiVendor::performDeactivate ()
{
	SwgCuiAuctionListBase::performDeactivate ();
}

//======================================================================
