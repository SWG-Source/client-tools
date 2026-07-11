// ======================================================================
//
// AuctionDataArchive.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "AuctionDataArchive.h"

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "AuctionData.h"

namespace Archive
{
	void get(ReadIterator& source, Auction::PalettizedItemDataHeader& target)
	{
		get(source, target.itemId);
		get(source, target.itemNameKey);
		get(source, target.highBid);
		get(source, target.timer);
		get(source, target.buyNowPrice);
		get(source, target.locationKey);
		get(source, target.ownerId);
		get(source, target.ownerNameKey);
		get(source, target.highBidderId);
		get(source, target.highBidderNameKey);
		get(source, target.maxProxyBid);
		get(source, target.myBid);
		get(source, target.itemType);
		get(source, target.resourceContainerClassCrc);
		get(source, target.flags);
		get(source, target.entranceCharge);
	}

	void put(ByteStream& target, Auction::PalettizedItemDataHeader const& source)
	{
		put(target, source.itemId);
		put(target, source.itemNameKey);
		put(target, source.highBid);
		put(target, source.timer);
		put(target, source.buyNowPrice);
		put(target, source.locationKey);
		put(target, source.ownerId);
		put(target, source.ownerNameKey);
		put(target, source.highBidderId);
		put(target, source.highBidderNameKey);
		put(target, source.maxProxyBid);
		put(target, source.myBid);
		put(target, source.itemType);
		put(target, source.resourceContainerClassCrc);
		put(target, source.flags);
		put(target, source.entranceCharge);
	}

	void get(ReadIterator& source, Auction::ItemDataDetails& target)
	{
		get(source, target.itemId);
		get(source, target.userDescription);
		get(source, target.propertyList);
		get(source, target.templateName);
		get(source, target.appearanceString);
	}

	void put(ByteStream& target, Auction::ItemDataDetails const& source)
	{
		put(target, source.itemId);
		put(target, source.userDescription);
		put(target, source.propertyList);
		put(target, source.templateName);
		put(target, source.appearanceString);
	}
}
