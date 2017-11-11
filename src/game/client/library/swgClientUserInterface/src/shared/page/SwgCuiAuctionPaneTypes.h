//======================================================================
//
// SwgCuiAuctionPaneTypes.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiAuctionPaneTypes_H
#define INCLUDED_SwgCuiAuctionPaneTypes_H

//======================================================================

namespace SwgCuiAuctionPaneTypes
{
	// SwgCuiAuctionPaneTypes::Type and CuiPreferences::ms_cscs *MUST* be kept in sync
	enum Type
	{
		T_all,
		T_mySales,
		T_myBids,
		T_available,
		T_vendorSellerSelling,
		T_vendorSellerOffers,
		T_vendorSellerStockroom,
		T_vendorBuyerSelling,
		T_vendorBuyerOffers,
		T_location,
		T_numTypes
	};

	int getAuctionListType (int paneType);
}

//======================================================================

#endif
