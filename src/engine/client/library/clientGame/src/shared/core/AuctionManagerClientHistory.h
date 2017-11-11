//======================================================================
//
// AuctionManagerClientHistory.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_AuctionManagerClientHistory_H
#define INCLUDED_AuctionManagerClientHistory_H

//======================================================================

class NetworkId;

//----------------------------------------------------------------------

class AuctionManagerClientHistory
{
public:
	static void                   bidAdd       (const NetworkId & itemId, bool isBuying);
	static bool                   bidRemove    (const NetworkId & itemId, bool & isBuying);

	static void                   saleAdd      (const NetworkId & itemId, bool isAuction);
	static bool                   saleRemove    (const NetworkId & itemId, bool & isAuction);

};

//======================================================================

#endif
