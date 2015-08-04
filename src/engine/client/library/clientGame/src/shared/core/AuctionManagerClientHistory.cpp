//======================================================================
//
// AuctionManagerClientHistory.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/AuctionManagerClientHistory.h"

#include "sharedFoundation/NetworkId.h"

#include <map>

//======================================================================

namespace
{
	typedef stdmap<NetworkId, bool>::fwd   BoolMap;
	
	namespace Maps
	{
		BoolMap      bid;
		BoolMap      sale;
	}

	template <typename T> T removeFromMap (std::map<NetworkId, T> & theMap, const NetworkId & itemId, T & ret)
	{
		const stdmap<NetworkId, T>::fwd::iterator it = theMap.find (itemId);
		
		if (it != theMap.end ())
		{
			ret = (*it).second;
			theMap.erase (it);
			return true;
		}
		
		return false;
	}
}

//----------------------------------------------------------------------

void AuctionManagerClientHistory::bidAdd       (const NetworkId & itemId, bool isBuying)
{
	Maps::bid [itemId] = isBuying;
}

//----------------------------------------------------------------------

bool AuctionManagerClientHistory::bidRemove    (const NetworkId & itemId, bool & isBuying)
{
	return removeFromMap<bool> (Maps::bid, itemId, isBuying);
}

//----------------------------------------------------------------------

void AuctionManagerClientHistory::saleAdd      (const NetworkId & itemId, bool isAuction)
{
	Maps::sale [itemId] = isAuction;
}

//----------------------------------------------------------------------

bool AuctionManagerClientHistory::saleRemove    (const NetworkId & itemId, bool & isAuction)
{
	return removeFromMap<bool> (Maps::sale, itemId, isAuction);
}

//======================================================================
