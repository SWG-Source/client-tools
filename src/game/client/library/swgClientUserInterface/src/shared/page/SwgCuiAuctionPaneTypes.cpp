//======================================================================
//
// SwgCuiAuctionPaneTypes.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAuctionPaneTypes.h"

#include "clientGame/AuctionManagerClient.h"

//======================================================================

namespace SwgCuiAuctionPaneTypes
{
	int getAuctionListType (int paneType)
	{
		switch (paneType)
		{
		case T_all:
			return AuctionManagerClient::T_all;
		case T_mySales:
			return AuctionManagerClient::T_mySales;
		case T_myBids:
			return AuctionManagerClient::T_myBids;
		case T_available:
			return AuctionManagerClient::T_available;
		case T_vendorBuyerSelling:
		case T_vendorSellerSelling:
			return AuctionManagerClient::T_vendorSelling;
		case T_vendorBuyerOffers:
		case T_vendorSellerOffers:
			return AuctionManagerClient::T_vendorOffers;
		case T_vendorSellerStockroom:
			return AuctionManagerClient::T_vendorStockroom;
		case T_location:
			return AuctionManagerClient::T_location;
		case T_numTypes:
			DEBUG_FATAL (true, ("SwgCuiAuctionPaneTypes invalid paneType T_numTypes"));
		}
		
		DEBUG_FATAL (true, ("SwgCuiAuctionPaneTypes invalid paneType [%d]", paneType));
		return -1;
	}
}
		
//======================================================================
